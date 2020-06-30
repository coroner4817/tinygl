#include "tinygl/glApp.h"
#include "tinygl/camera.h"
#include "tinygl/shader_s.h"
#include "tinygl/default_mesh.h"

int main()
{
  GLApp app;
  app.init();
  app.setProjectionMatrix(45.f, 0.1f, 100.f);
  Camera cam;
  cam.addViewMatrixUpdateListener(&app);
  DefaultMesh dm;
  dm.initPlaneData();

  app.loadTexture2D(ASSETS_2D_BASEPATH + "/wood.png", 1);

  Shader phongShader(getExecutablePath() + "/phong.vs", getExecutablePath() + "/phong.fs");
  phongShader.init("model", "view", &cam, "projection", app.getProjectionMatrix());
  phongShader.setInt("texture1", 1);
  phongShader.setVec3("lightPos", {0, 0, 0});
  phongShader.setInt("blinn", true);
  phongShader.setModelMatrix(glm::vec3(0, -0.5, 0), glm::vec3(1), 0, glm::vec3());

  app.generateNewVAOFromArray("plane", dm.getPlaneVerticesArray(), dm.getPlaneVerticesArraySize(), {{0, 3}, {1, 2}, {2, 3}}, dm.getPlaneIndicesArray(), dm.getPlaneIndicesArraySize());

  app.run([=, &phongShader, &cam, &app, &dm](float a_delta){
    cam.update(app.getWindow(), a_delta);

    // plane
    phongShader.use();
    app.useVAO("plane");
    phongShader.setVec3("viewPos", cam.GetCameraPos());
    glDrawElements(GL_TRIANGLES, dm.getPlaneVerticesCount(), GL_UNSIGNED_INT, 0);
  });
}