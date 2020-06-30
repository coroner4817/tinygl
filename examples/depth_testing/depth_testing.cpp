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

  Shader objShader(getExecutablePath() + "/depth_testing.vs", getExecutablePath() + "/depth_testing.fs");
  objShader.use();
  glm::mat4 model = glm::mat4(1.0f);
  objShader.setMatrix4fv("model", model);
  objShader.setMatrix4fv("view", cam.GetViewMatrix());
  objShader.setMatrix4fv("projection", app.getProjectionMatrix());

  app.generateNewVAOFromArray("objCube", dm.getCubeVerticesArray(), dm.getCubeVerticesArraySize(), {{0, 3}, {1, 2}, {2, 3}}, dm.getCubeIndicesArray(), dm.getCubeIndicesArraySize());
  app.generateNewVAOFromArray("plane", dm.getPlaneVerticesArray(), dm.getPlaneVerticesArraySize(), {{0, 3}, {1, 2}, {2, 3}}, dm.getPlaneIndicesArray(), dm.getPlaneIndicesArraySize());

  app.run([=, &objShader, &cam, &app, &dm](float a_delta){
    cam.update(app.getWindow(), a_delta);

    objShader.use();

    // plane
    app.useVAO("plane");
    objShader.setInt("texture1", 1);
    glm::mat4 model = glm::mat4(1.0f);
    objShader.setMatrix4fv("model", model);
    glDrawElements(GL_TRIANGLES, dm.getPlaneVerticesCount(), GL_UNSIGNED_INT, 0);

    // cubes
    app.useVAO("objCube");
    objShader.setInt("texture1", 0);
    for(int i = 0; i < cudeNum; ++i){
      glm::mat4 model = glm::mat4(1.0f);
      model = glm::translate(model, cubePositions[i]);
      objShader.setMatrix4fv("model", model);
      objShader.setMatrix4fv("view", cam.GetViewMatrix());
      glDrawElements(GL_TRIANGLES, dm.getCubeVerticesCount(), GL_UNSIGNED_INT, 0);
    }

  });
}