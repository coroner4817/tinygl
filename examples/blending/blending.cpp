#include "tinygl/glApp.h"
#include "tinygl/camera.h"
#include "tinygl/shader_s.h"
#include "tinygl/default_mesh.h"

int main()
{
  // ----------------------------------------------------------
  // user define data
  int cudeNum = 2;
  glm::vec3 cubePositions[] = {
    glm::vec3( -1.0f,  0.0f,  -1.0f),
    glm::vec3( 2.0f,  0.0f, 0.0f),
  };
  std::vector<glm::vec3> vegetation;
  vegetation.push_back(glm::vec3(-1.0f,  0.0f, -0.48f));
  vegetation.push_back(glm::vec3( 2.0f,  0.0f,  0.51f));
  vegetation.push_back(glm::vec3( 0.0f,  0.0f,  0.7f));
  vegetation.push_back(glm::vec3(-0.3f,  0.0f, -2.3f));
  vegetation.push_back(glm::vec3( 0.5f,  0.0f, -0.6f));
  // ----------------------------------------------------------

  GLApp app;
  app.init();
  app.setProjectionMatrix(45.f, 0.1f, 100.f);
  Camera cam;
  cam.addViewMatrixUpdateListener(&app);
  DefaultMesh dm;
  dm.initPlaneData();
  dm.initCubeData();

  app.loadTexture2D(ASSETS_2D_BASEPATH + "/marble.jpg", 0);
  app.loadTexture2D(ASSETS_2D_BASEPATH + "/metal.png", 1);
  app.loadTexture2D(ASSETS_2D_BASEPATH + "/grass.png", 2);
  app.loadTexture2D(ASSETS_2D_BASEPATH + "/blending_transparent_window.png", 3);

  Shader stdShader(COMMON_SHADERS_BASEPATH + "/stdShader.vs", COMMON_SHADERS_BASEPATH + "/stdShader.fs");
  stdShader.init("model", "view", &cam, "projection", app.getProjectionMatrix());
  stdShader.setInt("isColor", false);
  Shader objShader(getExecutablePath() + "/objShader.vs", getExecutablePath() + "/objShader.fs");
  objShader.init("model", "view", &cam, "projection", app.getProjectionMatrix());

  app.generateNewVAOFromArray("objCube", dm.getCubeVerticesArray(), dm.getCubeVerticesArraySize(), {{0, 3}, {1, 2}, {2, 3}}, dm.getCubeIndicesArray(), dm.getCubeIndicesArraySize());
  app.generateNewVAOFromArray("plane", dm.getPlaneVerticesArray(), dm.getPlaneVerticesArraySize(), {{0, 3}, {1, 2}, {2, 3}}, dm.getPlaneIndicesArray(), dm.getPlaneIndicesArraySize());

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  app.run([=, &objShader, &stdShader, &cam, &app, &dm, &vegetation](float a_delta){
    cam.update(app.getWindow(), a_delta);

    // plane
    stdShader.use();
    app.useVAO("plane");
    stdShader.setInt("texture1", 1);
    stdShader.setModelMatrix(glm::vec3(0, -0.5, 0), glm::vec3(1), 0, glm::vec3());
    glDrawElements(GL_TRIANGLES, dm.getPlaneVerticesCount(), GL_UNSIGNED_INT, 0);

    // cubes
    stdShader.use();
    app.useVAO("objCube");
    stdShader.setInt("texture1", 0);
    for(int i = 0; i < cudeNum; ++i){
      stdShader.setModelMatrix(cubePositions[i], glm::vec3(1), 0, glm::vec3());
      glDrawElements(GL_TRIANGLES, dm.getCubeVerticesCount(), GL_UNSIGNED_INT, 0);
    }

    // grass
    objShader.use();
    app.useVAO("plane");
    objShader.setInt("texture1", 2);
    for(int i = 0; i < vegetation.size(); ++i){
      objShader.setModelMatrix(vegetation[i], glm::vec3(0.1), M_PI/2, glm::vec3(1, 0, 0));
      glDrawElements(GL_TRIANGLES, dm.getPlaneVerticesCount(), GL_UNSIGNED_INT, 0);
    }

    // mask
    stdShader.use();
    app.useVAO("plane");
    stdShader.setInt("texture1", 3);
    // need to sort here so that for the transparent scene, the back get draw first. Then there is no depth issue when draw front
    // here will sort the original vec everytime.
    // glm::length return the length before square root which is enough for comparasion
    std::sort(vegetation.begin(), vegetation.end(), [&cam](const glm::vec3& pos1, const glm::vec3& pos2) -> bool {
      return glm::length2(cam.GetCameraPos() - pos1) >= glm::length2(cam.GetCameraPos() - pos2);
    });
    for(int i = 0; i < vegetation.size(); ++i){
      stdShader.setModelMatrix(vegetation[i] + glm::vec3(0, 0, 0.1), glm::vec3(0.1), M_PI/2, glm::vec3(1, 0, 0));
      glDrawElements(GL_TRIANGLES, dm.getPlaneVerticesCount(), GL_UNSIGNED_INT, 0);
    }
  });
}