#include "tinygl/glApp.h"
#include "tinygl/camera.h"
#include "tinygl/shader_s.h"
#include "tinygl/default_mesh.h"

int main()
{
  // lighting info
  // -------------
  glm::vec3 lightPositions[] = {
      glm::vec3(-3.0f, 0.0f, 0.0f),
      glm::vec3(-1.0f, 0.0f, 0.0f),
      glm::vec3 (1.0f, 0.0f, 0.0f),
      glm::vec3 (3.0f, 0.0f, 0.0f)
  };
  glm::vec3 lightColors[] = {
      glm::vec3(0.25),
      glm::vec3(0.50),
      glm::vec3(0.75),
      glm::vec3(1.00)
  };

  GLApp app;
  app.init();
  app.setProjectionMatrix(45.f, 0.1f, 100.f);
  Camera cam;
  cam.addViewMatrixUpdateListener(&app);
  DefaultMesh dm;
  dm.initPlaneData();

  app.loadTexture2D(ASSETS_2D_BASEPATH + "/wood.png", 1, true);

  Shader gammaShader(getExecutablePath() + "/gamma.vs", getExecutablePath() + "/gamma.fs");
  gammaShader.init("model", "view", &cam, "projection", app.getProjectionMatrix());
  gammaShader.setInt("texture1", 1);
  gammaShader.setInt("gamma", true);
  gammaShader.setModelMatrix(glm::vec3(0, -0.5, 0), glm::vec3(1), 0, glm::vec3());

  app.generateNewVAOFromArray("plane", dm.getPlaneVerticesArray(), dm.getPlaneVerticesArraySize(), {{0, 3}, {1, 2}, {2, 3}}, dm.getPlaneIndicesArray(), dm.getPlaneIndicesArraySize());

  app.run([=, &gammaShader, &cam, &app, &dm](float a_delta){
    cam.update(app.getWindow(), a_delta);

    // plane
    gammaShader.use();
    // set light array uniforms
    glUniform3fv(glGetUniformLocation(gammaShader.ID, "lightPositions"), 4, &lightPositions[0][0]);
    glUniform3fv(glGetUniformLocation(gammaShader.ID, "lightColors"), 4, &lightColors[0][0]);

    app.useVAO("plane");
    glDrawElements(GL_TRIANGLES, dm.getPlaneVerticesCount(), GL_UNSIGNED_INT, 0);
  });
}