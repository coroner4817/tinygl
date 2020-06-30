#include "tinygl/glApp.h"
#include "tinygl/camera.h"
#include "tinygl/shader_s.h"
#include "tinygl/default_mesh.h"

glm::vec3 lightPositions[] = {
  glm::vec3(-10.0f,  10.0f, 10.0f),
  glm::vec3( 10.0f,  10.0f, 10.0f),
  glm::vec3(-10.0f, -10.0f, 10.0f),
  glm::vec3( 10.0f, -10.0f, 10.0f),
};
glm::vec3 lightColors[] = {
  glm::vec3(300.0f, 300.0f, 300.0f),
  glm::vec3(300.0f, 300.0f, 300.0f),
  glm::vec3(300.0f, 300.0f, 300.0f),
  glm::vec3(300.0f, 300.0f, 300.0f)
};
int nrRows    = 7;
int nrColumns = 7;
float spacing = 2.5;

int main()
{
  GLApp app;
  app.init();
  app.setProjectionMatrix(45.f, 0.1f, 100.f);
  Camera cam;
  cam.addViewMatrixUpdateListener(&app);
  DefaultMesh dm;
  dm.initSphereData2();
  app.generateNewVAOFromArray("sphere", dm.getSphereVerticesArray(), dm.getSphereVerticesArraySize(), {{0, 3}, {1, 2}, {2, 3}}, dm.getSphereIndicesArray(), dm.getSphereIndicesArraySize());

  app.loadTexture2D(ASSETS_PBR_BASEPATH + "/rusted_iron/albedo.png", 0);
  app.loadTexture2D(ASSETS_PBR_BASEPATH + "/rusted_iron/normal.png", 1);
  app.loadTexture2D(ASSETS_PBR_BASEPATH + "/rusted_iron/metallic.png", 2);
  app.loadTexture2D(ASSETS_PBR_BASEPATH + "/rusted_iron/roughness.png", 3);
  app.loadTexture2D(ASSETS_PBR_BASEPATH + "/rusted_iron/ao.png", 4);

  Shader pbrShader(getExecutablePath() + "/pbr.vs", getExecutablePath() + "/pbr.fs");
  pbrShader.init("model", "view", &cam, "projection", app.getProjectionMatrix(), "camPos");
  pbrShader.setInt("albedoMap", 0);
  pbrShader.setInt("normalMap", 1);
  pbrShader.setInt("metallicMap", 2);
  pbrShader.setInt("roughnessMap", 3);
  pbrShader.setInt("aoMap", 4);

  app.run([=, &pbrShader, &cam, &app, &dm](float a_delta){
    cam.update(app.getWindow(), a_delta);

    pbrShader.use();{
      app.useVAO("sphere");

      // setup light
      for (unsigned int i = 0; i < sizeof(lightPositions) / sizeof(lightPositions[0]); ++i)
      {
        glm::vec3 newPos = lightPositions[i] + glm::vec3(sin(glfwGetTime() * 5.0) * 5.0, 0.0, 0.0); // far away
        newPos = lightPositions[i];
        pbrShader.setVec3("lightPositions[" + std::to_string(i) + "]", newPos);
        pbrShader.setVec3("lightColors[" + std::to_string(i) + "]", lightColors[i]);
      }

      // render rows*column number of spheres with varying metallic/roughness values scaled by rows and columns respectively
      glm::mat4 model = glm::mat4(1.0f);
      for (int row = 0; row < nrRows; ++row)
      {
        for (int col = 0; col < nrColumns; ++col)
        {
          // we clamp the roughness to 0.025 - 1.0 as perfectly smooth surfaces (roughness of 0.0) tend to look a bit off
          // on direct lighting.
          pbrShader.setModelMatrix(glm::vec3((col - (nrColumns / 2)) * spacing, (row - (nrRows / 2)) * spacing, 0.0f));
          glDrawElements(GL_TRIANGLE_STRIP, dm.getSphereVerticesCount(), GL_UNSIGNED_INT, 0);
        }
      }
    }
  });
}