#include "tinygl/glApp.h"
#include "tinygl/camera.h"
#include "tinygl/shader_s.h"
#include "tinygl/default_mesh.h"

// ----------------------------------------------------------
// user define data
glm::vec3 cubePositions[] = {
  glm::vec3( 0.0f,  1.5f,  0.0f),
  glm::vec3( 2.0f,  0.0f, 1.0f),
  glm::vec3( -1.0f,  0.0f, 2.0f),
};
glm::vec3 lightPos(-2.0f, 4.0f, -1.0f);
// ----------------------------------------------------------

void renderScene(Shader& shader, GLApp& app, DefaultMesh& dm);

int main()
{
  GLApp app;
  app.init();
  app.setProjectionMatrix(45.f, 0.1f, 100.f);
  Camera cam;
  cam.addViewMatrixUpdateListener(&app);
  DefaultMesh dm;
  dm.initPlaneData();
  dm.initCubeData();
  dm.initQuadData();

  app.loadTexture2D(ASSETS_2D_BASEPATH + "/wood.png", 1);
  app.loadTexture2D(ASSETS_2D_BASEPATH + "/container.jpg", 2);

  Shader depthShader(getExecutablePath() + "/depthShader.vs", getExecutablePath() + "/depthShader.fs");
  depthShader.init("model");
  Shader shadowShader(getExecutablePath() + "/shadowShader.vs", getExecutablePath() + "/shadowShader.fs");
  shadowShader.init("model", "view", &cam, "projection", app.getProjectionMatrix(), "cameraPos");
  Shader debugShader(getExecutablePath() + "/debug.vs", getExecutablePath() + "/debug.fs");

  app.generateNewVAOFromArray("objCube", dm.getCubeVerticesArray(), dm.getCubeVerticesArraySize(), {{0, 3}, {1, 2}, {2, 3}}, dm.getCubeIndicesArray(), dm.getCubeIndicesArraySize());
  app.generateNewVAOFromArray("plane", dm.getPlaneVerticesArray(), dm.getPlaneVerticesArraySize(), {{0, 3}, {1, 2}, {2, 3}}, dm.getPlaneIndicesArray(), dm.getPlaneIndicesArraySize());
  app.generateNewVAOFromArray("quad", dm.getQuadVerticesArray(), dm.getQuadVerticesArraySize(), {{0, 3}, {1, 2}}, dm.getQuadIndicesArray(), dm.getQuadIndicesArraySize());

  // configure depth map FBO
  // shadow might have a different resolution than the actual content
  // because we only care about the depth here
  const unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;
  unsigned int depthMapFBO;
  glGenFramebuffers(1, &depthMapFBO);
  // create depth texture
  unsigned int depthMap;
  glGenTextures(1, &depthMap);
  glBindTexture(GL_TEXTURE_2D, depthMap);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  // use GL_CLAMP_TO_BORDER so that for the fragment outside of the light projection frustum
  // all the depth value are 1
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
  float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
  glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
  // attach depth texture as FBO's depth buffer
  glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
  // set the color buffer and read buffer not available
  glDrawBuffer(GL_NONE);
  glReadBuffer(GL_NONE);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  // tf for transform world coordinate to light space
  glm::mat4 lightProjection, lightView;
  glm::mat4 lightSpaceMatrix;
  float near_plane = 1.0f, far_plane = 7.5f;
  // simple projection matrix from the light POV
  lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane);
  // static light view matrix
  lightView = glm::lookAt(lightPos, glm::vec3(0.0f), glm::vec3(0.0, 1.0, 0.0));
  lightSpaceMatrix = lightProjection * lightView;

  depthShader.use();
  depthShader.setMatrix4fv("lightSpaceMatrix", lightSpaceMatrix);

  shadowShader.use();
  shadowShader.setMatrix4fv("lightSpaceMatrix", lightSpaceMatrix);
  shadowShader.setVec3("lightPos", lightPos);
  // cannot use my texture manager here, because the shadowMap is created
  // in the FBO, it doesn't pass the texture manager registration.
  // so have to manually set to channel 0 and active channel 0 in the run code
  shadowShader.setInt("shadowMap", 0);

  debugShader.use();
  // use channel 0 which is also used on the second pass
  debugShader.setInt("depthMap", 0);

  app.run([=, &depthShader, &shadowShader, &debugShader, &cam, &app, &dm](float a_delta){
    cam.update(app.getWindow(), a_delta);
    // first pass
    // shadow map is smaller than actual content
    // ViewPort doesn't affect the content that is rendered!!!
    glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
    // write depth info to our FBO
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    // don't clear GL_COLOR_BUFFER_BIT so that user doesn't see black frame while processing this pass
    glClear(GL_DEPTH_BUFFER_BIT);
    // solve the peter panning issue by using the back face to determine the depth map
    glCullFace(GL_FRONT);
    renderScene(depthShader, app, dm);
    glCullFace(GL_BACK); // reset to default
    // bind FBO back to the default FBO for rendering
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    // after first pass, we store the frame depth map in the depthMap texture buffer
    // the depth map is in the light POV space

    // second pass actual rendering
    { // render simple shadow
      glViewport(0, 0, 1280, 720);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      shadowShader.use();
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, depthMap);
      // when applying shadow, we are applying the global depth map individually to each VAO
      renderScene(shadowShader, app, dm);
    }

    { // debug pass rendering to visualize the depth buffer
      // // reset viewport
      // glViewport(0, 0, 1280, 720);
      // glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      // // render on the debug quad
      // debugShader.use();
      // app.useVAO("quad");
      // debugShader.setFloat("near_plane", near_plane);
      // debugShader.setFloat("far_plane", far_plane);
      // glActiveTexture(GL_TEXTURE0);
      // // bind to the depth texture in the framebuffer
      // glBindTexture(GL_TEXTURE_2D, depthMap);
      // glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
      // glBindVertexArray(0);
    }
  });
}

void renderScene(Shader& shader, GLApp& app, DefaultMesh& dm)
{
  // plane
  shader.use();
  app.useVAO("plane");
  // for the second actual render pass
  shader.setInt("diffuseTexture", 1);
  shader.setModelMatrix(glm::vec3(0, -0.26, 0), glm::vec3(3), 0, glm::vec3());
  glDrawElements(GL_TRIANGLES, dm.getPlaneVerticesCount(), GL_UNSIGNED_INT, 0);

  // cubes
  shader.use();
  app.useVAO("objCube");
  // for the second actual render pass
  shader.setInt("diffuseTexture", 2);
  shader.setModelMatrix(cubePositions[0], glm::vec3(0.5), 0, glm::vec3());
  glDrawElements(GL_TRIANGLES, dm.getCubeVerticesCount(), GL_UNSIGNED_INT, 0);
  shader.setModelMatrix(cubePositions[1], glm::vec3(0.5), 0, glm::vec3());
  glDrawElements(GL_TRIANGLES, dm.getCubeVerticesCount(), GL_UNSIGNED_INT, 0);
  shader.setModelMatrix(cubePositions[2], glm::vec3(0.25), glm::radians(60.f), glm::normalize(glm::vec3(1, 0, 1)));
  glDrawElements(GL_TRIANGLES, dm.getCubeVerticesCount(), GL_UNSIGNED_INT, 0);
}