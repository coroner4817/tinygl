#include "tinygl/glApp.h"
#include "tinygl/camera.h"
#include "tinygl/shader_s.h"
#include "tinygl/default_mesh.h"

// framebuffer is the buffer of the whole gl window
// it hold the window level buffer like color buffer, depth buffer and stencil buffer
// by default the glfw will create a default framebuffer for us.
int main()
{
  // ----------------------------------------------------------
  // user define data
  int cudeNum = 2;
  glm::vec3 cubePositions[] = {
    glm::vec3( -1.0f,  0.0f,  -1.0f),
    glm::vec3( 2.0f,  0.0f, 0.0f),
  };
  float quadVertices[] = { // vertex attributes for a quad that fills the entire screen in Normalized Device Coordinates.
    // positions   // texCoords
    -1.0f,  1.0f,  0.0f, 1.0f,
    -1.0f, -1.0f,  0.0f, 0.0f,
    1.0f, -1.0f,  1.0f, 0.0f,

    -1.0f,  1.0f,  0.0f, 1.0f,
    1.0f, -1.0f,  1.0f, 0.0f,
    1.0f,  1.0f,  1.0f, 1.0f
  };
  unsigned int quadIndices[] = {
    0, 1, 2,
    3, 4, 5
  };
  // ----------------------------------------------------------

  GLApp app;
  app.init();
  app.setProjectionMatrix(45.f, 0.1f, 100.f);
  Camera cam;
  cam.addViewMatrixUpdateListener(&app);
  DefaultMesh dm;
  dm.initPlaneData();
  dm.initCubeData();

  app.loadTexture2D(ASSETS_2D_BASEPATH + "/container.jpg", 1);
  app.loadTexture2D(ASSETS_2D_BASEPATH + "/metal.png", 2);

  Shader stdShader(COMMON_SHADERS_BASEPATH + "/stdShader.vs", COMMON_SHADERS_BASEPATH + "/stdShader.fs");
  stdShader.init("model", "view", &cam, "projection", app.getProjectionMatrix());
  stdShader->setInt("isColor", false);
  Shader screenShader(getExecutablePath() + "/screenShader.vs", getExecutablePath() + "/screenShader.fs");

  app.generateNewVAOFromArray("objCube", dm.getCubeVerticesArray(), dm.getCubeVerticesArraySize(), {{0, 3}, {1, 2}, {2, 3}}, dm.getCubeIndicesArray(), dm.getCubeIndicesArraySize());
  app.generateNewVAOFromArray("plane", dm.getPlaneVerticesArray(), dm.getPlaneVerticesArraySize(), {{0, 3}, {1, 2}, {2, 3}}, dm.getPlaneIndicesArray(), dm.getPlaneIndicesArraySize());
  app.generateNewVAOFromArray("quad", quadVertices, sizeof(quadVertices), {{0, 2}, {1, 2}}, quadIndices, sizeof(quadIndices));

  // uncomment this to draw in wire frame mode
  // should show a quad wireframe, because in the defualt framebuffer the whole scene rendered as a quad image
  // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

  // this temprary framebuffer is just as cache buffer of the scene
  // the only default buffer is still the framebuffer 0
  // textureColorbuffer cache the scene's color buffer on the window
  // rbo store the depth&stencil buffer for the scene
  unsigned int framebuffer, textureColorbuffer, rbo;
  app.genFrameBuffer(framebuffer, textureColorbuffer, rbo);

  app.run([=, &stdShader, &screenShader, &cam, &app, &dm](float a_delta){
    cam.update(app.getWindow(), a_delta);

    // bind to framebuffer and draw scene as we normally would to color texture
    // the scene is rendered in the this framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glEnable(GL_DEPTH_TEST); // enable depth testing (is disabled for rendering screen-space quad)

    // make sure we clear the framebuffer's content
    // mandatory to clean up here because the glClear in the update loop is clean the default framebuffer not this generated framebuffer
    app.glFrameBufferClear();

    // plane
    stdShader.use();
    app.useVAO("plane");
    stdShader.setInt("texture1", 2);
    stdShader.setModelMatrix(glm::vec3(0, -0.51, 0), glm::vec3(1), 0, glm::vec3());
    glDrawElements(GL_TRIANGLES, dm.getPlaneVerticesCount(), GL_UNSIGNED_INT, 0);

    // cubes
    stdShader.use();
    app.useVAO("objCube");
    stdShader.setInt("texture1", 1);
    for(int i = 0; i < cudeNum; ++i){
      stdShader.setModelMatrix(cubePositions[i], glm::vec3(1), 0, glm::vec3());
      glDrawElements(GL_TRIANGLES, dm.getCubeVerticesCount(), GL_UNSIGNED_INT, 0);
    }

    // now bind back to default framebuffer and draw a quad plane with the attached framebuffer color texture
    // here we use the default framebuffer which just a quad, to overlay the framebuffer we declared before, which draw the plane and the boxes
    // so in this demo you should see the whole scene(plane and boxes) be rendered as a single image
    // if comment out following, you will see nothing, because default framebuffer is not filled
    // because there is 2 framebuffer need to be filled, so there is a little bit delay
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    // disable depth test so screen-space quad isn't discarded due to depth test.
    // the depth test will still test across different framebuffers
    glDisable(GL_DEPTH_TEST);
    // this default buffer is already cleared by the glApp update loop, so no need to clear again
    // these clear color is just for visualize the wire frame, uncomment with glPolygonMode
    // glClearColor(1.0f, 1.0f, 1.0f, 1.0f); // set clear color to white (not really necessery actually, since we won't be able to see behind the quad anyways)
    // glClear(GL_COLOR_BUFFER_BIT);

    screenShader.use();
    app.useVAO("quad");
    screenShader.setInt("screenTexture", 0);
    // use the color attachment texture as the texture of the quad plane
    // this textureColorbuffer is filled in the above call, so that the whole scene is rendered as a single image
    // then we can apply image filter to the whole scene
    glBindTexture(GL_TEXTURE_2D, textureColorbuffer);
    glDrawArrays(GL_TRIANGLES, 0, 6);
  });
}