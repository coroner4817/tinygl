#include "tinygl/glApp.h"
#include "tinygl/camera.h"
#include "tinygl/shader_s.h"
#include "tinygl/default_mesh.h"

// ----------------------------------------------------------
// user define data
glm::vec3 lightPos(0.0f, 0.0f, 0.0f);
bool shadows = true;
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
  dm.initCubeData();

  app.loadTexture2D(ASSETS_2D_BASEPATH + "/wood.png", 1);
  app.loadTexture2D(ASSETS_2D_BASEPATH + "/container.jpg", 2);

  Shader depthShader(getExecutablePath() + "/depthShader.vs", getExecutablePath() + "/depthShader.fs", getExecutablePath() + "/depthShader.gs");
  depthShader.init("model");
  Shader shadowShader(getExecutablePath() + "/point_shadow.vs", getExecutablePath() + "/point_shadow.fs");
  shadowShader.init("model", "view", &cam, "projection", app.getProjectionMatrix(), "viewPos");

  app.generateNewVAOFromArray("objCube", dm.getCubeVerticesArray(), dm.getCubeVerticesArraySize(), {{0, 3}, {1, 2}, {2, 3}}, dm.getCubeIndicesArray(), dm.getCubeIndicesArraySize());

  // configure depth map FBO
  // -----------------------
  const unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;
  unsigned int depthMapFBO;
  glGenFramebuffers(1, &depthMapFBO);
  // create depth cubemap type texture
  unsigned int depthCubemap;
  glGenTextures(1, &depthCubemap);
  glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemap);
  for (unsigned int i = 0; i < 6; ++i)
      glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
  // attach depth texture as FBO's depth buffer
  glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
  glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthCubemap, 0);
  glDrawBuffer(GL_NONE);
  glReadBuffer(GL_NONE);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  shadowShader.use();
  shadowShader.setInt("depthMap", 0);

  app.run([=, &depthShader, &shadowShader, &cam, &app, &dm](float a_delta){
    cam.update(app.getWindow(), a_delta);

    // move light position over time
    lightPos.z = sin(glfwGetTime() * 0.5) * 3.0;
    // render
    // ------
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // 0. create depth cubemap transformation matrices
    // generate the light space tf matirx base on dynamic light position
    float near_plane = 1.0f;
    float far_plane  = 25.0f;
    glm::mat4 shadowProj = glm::perspective(glm::radians(90.0f), (float)SHADOW_WIDTH / (float)SHADOW_HEIGHT, near_plane, far_plane);
    // light space matirx for each face
    std::vector<glm::mat4> shadowTransforms;
    // we have the negative lookAt vec because opengl store texture data in a upside down way for GL_TEXTURE_CUBE_MAP typed texture buffer
    // https://stackoverflow.com/questions/11685608/convention-of-faces-in-opengl-cubemapping/11690553#11690553
    shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3( 1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)));
    shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)));
    shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3( 0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)));
    shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3( 0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)));
    shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3( 0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)));
    shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3( 0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f)));

    // 1. render scene to depth cubemap
    // --------------------------------
    glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glClear(GL_DEPTH_BUFFER_BIT);
    depthShader.use();
    for (unsigned int i = 0; i < 6; ++i)
        depthShader.setMatrix4fv("shadowMatrices[" + std::to_string(i) + "]", shadowTransforms[i]);
    depthShader.setFloat("far_plane", far_plane);
    depthShader.setVec3("lightPos", lightPos);
    renderScene(depthShader, app, dm);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // 2. render scene as normal
    // -------------------------
    app.resetViewport();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    shadowShader.use();
    shadowShader.setVec3("lightPos", lightPos);
    shadowShader.setInt("shadows", shadows); // enable/disable shadows by pressing 'SPACE'
    shadowShader.setFloat("far_plane", far_plane);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemap);
    renderScene(shadowShader, app, dm);
  });
}

void renderScene(Shader& shader, GLApp& app, DefaultMesh& dm)
{
  shader.use();
  app.useVAO("objCube");

  // room cube
  shader.setInt("diffuseTexture", 1);
  // doesn't have to because we didn't enable CULL_FACE at beginning, while in the lesson the author did
  // glDisable(GL_CULL_FACE); // note that we disable culling here since we render 'inside' the cube instead of the usual 'outside' which throws off the normal culling methods.
  shader.setInt("reverse_normals", 1); // A small little hack to invert normals when drawing cube from the inside so lighting still works.
  shader.setModelMatrix({0, 0, 0}, glm::vec3(10), 0, glm::vec3());
  glDrawElements(GL_TRIANGLES, dm.getCubeVerticesCount(), GL_UNSIGNED_INT, 0);
  shader.setInt("reverse_normals", 0); // and of course disable it
  // glEnable(GL_CULL_FACE);

  // cubes
  shader.setInt("diffuseTexture", 2); // for the second actual render pass
  shader.setModelMatrix({4.0f, -3.5f, 0.0}, glm::vec3(0.5), 0, glm::vec3());
  glDrawElements(GL_TRIANGLES, dm.getCubeVerticesCount(), GL_UNSIGNED_INT, 0);
  shader.setModelMatrix({2.0f, 3.0f, 1.0}, glm::vec3(0.75), 0, glm::vec3());
  glDrawElements(GL_TRIANGLES, dm.getCubeVerticesCount(), GL_UNSIGNED_INT, 0);
  shader.setModelMatrix({-3.0f, -1.0f, 0.0}, glm::vec3(0.5), 0, glm::vec3());
  glDrawElements(GL_TRIANGLES, dm.getCubeVerticesCount(), GL_UNSIGNED_INT, 0);
  shader.setModelMatrix({-1.5f, 1.0f, 1.5}, glm::vec3(0.5), 0, glm::vec3());
  glDrawElements(GL_TRIANGLES, dm.getCubeVerticesCount(), GL_UNSIGNED_INT, 0);
  shader.setModelMatrix({-1.5f, 2.0f, -3.0}, glm::vec3(0.75), glm::radians(60.f), glm::normalize(glm::vec3(1, 0, 1)));
  glDrawElements(GL_TRIANGLES, dm.getCubeVerticesCount(), GL_UNSIGNED_INT, 0);
}