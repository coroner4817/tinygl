#include "tinygl/glApp.h"
#include "tinygl/camera.h"
#include "tinygl/shader_s.h"
#include "tinygl/default_mesh.h"

// ----------------------------------------------------------
// user define data
glm::vec3 planePosition = glm::vec3(0, -1, 0);
std::vector<glm::vec3> cubePositions = {
  glm::vec3(0.0f, 1.5f, 0.0),
  glm::vec3(2.0f, 0.0f, 1.0),
  glm::vec3(-1.0f, 0.0f, 2.0),
  glm::vec3(0.0f, 2.7f, 4.0),
  glm::vec3(-2.0f, 1.0f, -3.0),
  glm::vec3(-3.0f, 0.0f, 0.0)
};
std::vector<glm::vec3> cubeScales = {
  glm::vec3(1.0f),
  glm::vec3(1.0f),
  glm::vec3(2.0f),
  glm::vec3(2.5f),
  glm::vec3(2.0f),
  glm::vec3(1.0f),
};
glm::vec3 lampScale = glm::vec3(0.5);
std::vector<glm::vec3> lightPositions = {
  glm::vec3( 0.5f, 0.5f,  1.5f),
  glm::vec3(-4.0f, 0.5f, -3.0f),
  glm::vec3( 3.0f, 0.5f,  1.0f),
  glm::vec3(-.8f,  2.4f, -1.0f)
};
std::vector<glm::vec3> lightColors = {
  glm::vec3(5.0f,   5.0f,  5.0f),
  glm::vec3(10.0f,  0.0f,  0.0f),
  glm::vec3(0.0f,   0.0f,  15.0f),
  glm::vec3(0.0f,   5.0f,  0.0f)
};
// ----------------------------------------------------------
// new way
int main()
{
  GLApp app;
  app.init();
  app.setProjectionMatrix(45.f, 0.1f, 100.f);
  Camera cam;
  cam.addViewMatrixUpdateListener(&app);
  DefaultMesh dm(&app, &cam);
  dm.setObjAttrComp(true);
  dm.initPlaneData();
  dm.initCubeData();
  dm.initQuadData();

  app.loadTexture2D(ASSETS_2D_BASEPATH + "/container2.png", 1);
  app.loadTexture2D(ASSETS_2D_BASEPATH + "/wood.png", 2);

  app.generateNewVAOFromArray("plane", dm.getPlaneVerticesArray(), dm.getPlaneVerticesArraySize(), {{0, 3}, {1, 3}, {2, 2}}, dm.getPlaneIndicesArray(), dm.getPlaneIndicesArraySize());
  app.generateNewVAOFromArray("cube", dm.getCubeVerticesArray(), dm.getCubeVerticesArraySize(), {{0, 3}, {1, 3}, {2, 2}}, dm.getCubeIndicesArray(), dm.getCubeIndicesArraySize());
  app.generateNewVAOFromArray("quad", dm.getQuadVerticesArray(), dm.getQuadVerticesArraySize(), {{0, 3}, {1, 2}}, dm.getQuadIndicesArray(), dm.getQuadIndicesArraySize());

  Shader sceneShader(getExecutablePath() + "/sceneShader.vs", getExecutablePath() + "/sceneShader.fs");
  sceneShader.init("model", "view", &cam, "projection", app.getProjectionMatrix(), "viewPos");
  for(int i = 0; i < 4; ++i){
    sceneShader.setVec3("lights[" + std::to_string(i) + "].Position", lightPositions[i]);
    sceneShader.setVec3("lights[" + std::to_string(i) + "].Color", lightColors[i]);
  }

  Shader blurShader(getExecutablePath() + "/blurShader.vs", getExecutablePath() + "/blurShader.fs");
  Shader frameShader(getExecutablePath() + "/frameShader.vs", getExecutablePath() + "/frameShader.fs");


  // Multiple Render Targets, render to 2 color attachment buffer
  unsigned int hdrFBO;
  glGenFramebuffers(1, &hdrFBO);
  glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);
  unsigned int colorBuffers[2];
  glGenTextures(2, colorBuffers);
  for (unsigned int i = 0; i < 2; i++)
  {
      glBindTexture(GL_TEXTURE_2D, colorBuffers[i]);
      glTexImage2D(
          GL_TEXTURE_2D, 0, GL_RGB16F, 1280, 720, 0, GL_RGB, GL_FLOAT, NULL
      );
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
      // attach texture to framebuffer
      glFramebufferTexture2D(
          GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, colorBuffers[i], 0
      );
  }
  // have to add the depth buffer or will lose depth information
  // create and attach depth buffer (renderbuffer)
  unsigned int rboDepth;
  glGenRenderbuffers(1, &rboDepth);
  glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, 1280, 720);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);
  // tell OpenGL which color attachments we'll use (of this framebuffer) for rendering
  unsigned int attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
  glDrawBuffers(2, attachments);
  // finally check if framebuffer is complete
  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
      std::cout << "Framebuffer not complete!" << std::endl;
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  // for second pass to store the blur image
  // gen 2 framebuffer and 2 color buffer here
  // for the 2 pass gaussian blur, it works like the swap buffer
  // buffer 1 will store the blured buffer 0 and then buffer 0 store the blured buffer 1
  // check the blurShader
  unsigned int pingpongFBO[2];
  unsigned int pingpongBuffers[2]; // color buffer
  glGenFramebuffers(2, pingpongFBO);
  glGenTextures(2, pingpongBuffers);
  for (unsigned int i = 0; i < 2; i++)
  {
      glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[i]);
      glBindTexture(GL_TEXTURE_2D, pingpongBuffers[i]);
      glTexImage2D(
          GL_TEXTURE_2D, 0, GL_RGB16F, 1280, 720, 0, GL_RGB, GL_FLOAT, NULL
      );
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
      glFramebufferTexture2D(
          GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pingpongBuffers[i], 0
      );
  }
  glBindFramebuffer(GL_FRAMEBUFFER, 0);


  app.run([=, &cam, &app, &dm, &sceneShader, &blurShader, &frameShader](float a_delta){
    cam.update(app.getWindow(), a_delta);

    // 1, first pass - render scene
    sceneShader.use();{
      glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      // plane
      app.useVAO("plane");
      sceneShader.setModelMatrix(planePosition);
      sceneShader.setInt("diffuseTexture", 2);
      sceneShader.setInt("isColor", false);
      glDrawElements(GL_TRIANGLES, dm.getPlaneVerticesCount(), GL_UNSIGNED_INT, 0);

      // cubes
      for(int i = 0; i < cubePositions.size(); ++i){
        app.useVAO("cube");
        sceneShader.setModelMatrix(cubePositions[i], cubeScales[i]);
        sceneShader.setInt("diffuseTexture", 1);
        sceneShader.setInt("isColor", false);
        glDrawElements(GL_TRIANGLES, dm.getCubeVerticesCount(), GL_UNSIGNED_INT, 0);
      }

      // lights
      for(int i = 0; i < lightPositions.size(); ++i){
        app.useVAO("cube");
        sceneShader.setModelMatrix(lightPositions[i], lampScale);
        sceneShader.setInt("isColor", true);
        sceneShader.setVec3("color", lightColors[i]);
        glDrawElements(GL_TRIANGLES, dm.getCubeVerticesCount(), GL_UNSIGNED_INT, 0);
      }
      glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    // 2, second pass - blur frame
    bool horizontal = true;
    blurShader.use();{
      bool first_iteration = true; // at the first_it load the content from the last pass framebuffer
      int amount = 10;
      app.useVAO("quad");
      blurShader.setInt("image", 0);
      glActiveTexture(GL_TEXTURE0);
      for (unsigned int i = 0; i < amount; i++)
      {
          glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[horizontal]);
          blurShader.setInt("horizontal", horizontal);
          // bind the texture from the other buffer
          glBindTexture(GL_TEXTURE_2D, first_iteration ? colorBuffers[1] : pingpongBuffers[!horizontal]);
          glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
          horizontal = !horizontal; // swap buffer
          if (first_iteration) first_iteration = false;
      }
      glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    // 3, third pass - render final result
    frameShader.use();{
      app.useVAO("quad");
      frameShader.setInt("bloom", true);
      frameShader.setFloat("exposure", 0.1);
      // original scene
      frameShader.setInt("scene", 0);
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, colorBuffers[0]);
      // blured bloom effect
      frameShader.setInt("bloomBlur", 1);
      glActiveTexture(GL_TEXTURE1);
      glBindTexture(GL_TEXTURE_2D, pingpongBuffers[!horizontal]); // take the last blur result
      glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    }
  });
}
