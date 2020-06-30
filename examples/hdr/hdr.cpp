#include "tinygl/glApp.h"
#include "tinygl/camera.h"
#include "tinygl/shader_s.h"
#include "tinygl/default_mesh.h"

// ----------------------------------------------------------
// user define data
glm::vec3 lightPos[] = {
  glm::vec3(0.f, 0.f, -46.5f),
  glm::vec3(-1.4f, -1.9f, -9.0f),
  glm::vec3( 0.0f, -1.8f, -4.0f),
  glm::vec3(0.8f, -1.7f, -6.0f)
};
glm::vec3 lightColors[] = {
  glm::vec3(200.f, 200.f, 200.f),
  glm::vec3(0.1f, 0.f, 0.f),
  glm::vec3(0.f, 0.f, 0.2f),
  glm::vec3(0.f, 0.1f, 0.f)
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
  dm.initCubeData();
  dm.initQuadData();

  app.loadTexture2D(ASSETS_2D_BASEPATH + "/wood.png", 1, true);
  app.generateNewVAOFromArray("cube", dm.getCubeVerticesArray(), dm.getCubeVerticesArraySize(), {{0, 3}, {1, 2}, {2, 3}}, dm.getCubeIndicesArray(), dm.getCubeIndicesArraySize());
  app.generateNewVAOFromArray("quad", dm.getQuadVerticesArray(), dm.getQuadVerticesArraySize(), {{0, 3}, {1, 2}}, dm.getQuadIndicesArray(), dm.getQuadIndicesArraySize());

  Shader dnsShader(getExecutablePath() + "/lighting.vs", getExecutablePath() + "/lighting.fs");
  dnsShader.init("model", "view", &cam, "projection", app.getProjectionMatrix(), "cameraPos");
  // strange bug. Need to be twice larger or the backface normal is wrong
  dnsShader.setModelMatrix(glm::vec3(0, 0, -22), glm::vec3(5, 5, 55));
  dnsShader.setInt("diffuseMap", 1);
  for(int i = 0; i < 4; ++i){
    dnsShader.setVec3("lights[" + std::to_string(i) + "].position", lightPos[i]);
    dnsShader.setVec3("lights[" + std::to_string(i) + "].color", lightColors[i]);
  }

  Shader hdrShader(getExecutablePath() + "/hdr.vs", getExecutablePath() + "/hdr.fs");
  hdrShader.setInt("hdrBuffer", 0); // use channel 0 to attach framebuffer
  hdrShader.setInt("hdr", true);
  hdrShader.setFloat("exposure", 1.0f);

  // configure floating point framebuffer
  // ------------------------------------
  unsigned int hdrFBO;
  glGenFramebuffers(1, &hdrFBO);
  // create floating point color buffer
  unsigned int colorBuffer;
  glGenTextures(1, &colorBuffer);
  glBindTexture(GL_TEXTURE_2D, colorBuffer);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, 1280, 720, 0, GL_RGBA, GL_FLOAT, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  // create depth buffer (renderbuffer)
  unsigned int rboDepth;
  glGenRenderbuffers(1, &rboDepth);
  glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, 1280, 720);
  // attach buffers
  glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorBuffer, 0);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);
  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
      std::cout << "Framebuffer not complete!" << std::endl;
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  app.run([=, &dnsShader, &hdrShader, &cam, &app, &dm](float a_delta){
    cam.update(app.getWindow(), a_delta);

    // first pass to get the framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      // tunnel
      dnsShader.use();
      app.useVAO("cube");
      glDrawElements(GL_TRIANGLES, dm.getCubeVerticesCount(), GL_UNSIGNED_INT, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // second pass to apply HDR color remapping
    hdrShader.use();
    app.useVAO("quad");
    // use TEXTURE0 as framebuffer is using GL_COLOR_ATTACHMENT0
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, colorBuffer);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
  });
}
