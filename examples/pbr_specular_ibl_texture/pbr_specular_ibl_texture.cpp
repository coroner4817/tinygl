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
  app.useSkyBox("Newport_Loft", 0, true, 1, 2, 3);
  Camera cam;
  cam.addViewMatrixUpdateListener(&app);
  DefaultMesh dm;
  dm.initSphereData2();
  app.generateNewVAOFromArray("sphere", dm.getSphereVerticesArray(), dm.getSphereVerticesArraySize(), {{0, 3}, {1, 2}, {2, 3}}, dm.getSphereIndicesArray(), dm.getSphereIndicesArraySize());

  Shader pbrShader(COMMON_SHADERS_BASEPATH + "/pbr.vs", getExecutablePath() + "/pbr.fs");
  pbrShader.init("model", "view", &cam, "projection", app.getProjectionMatrix(), "camPos");
  pbrShader.setVec3("albedo", {0.5f, 0.0f, 0.0f});
  pbrShader.setFloat("ao", 1.0f);
  pbrShader.setInt("irradianceMap", 1);
  pbrShader.setInt("prefilterMap", 2);
  pbrShader.setInt("brdfLUT", 3);

  pbrShader.setInt("albedoMap", 4);
  pbrShader.setInt("normalMap", 5);
  pbrShader.setInt("metallicMap", 6);
  pbrShader.setInt("roughnessMap", 7);
  pbrShader.setInt("aoMap", 8);

  unsigned int ironAlbedoMap = app.loadTexture2DBase(ASSETS_PBR_BASEPATH + "/rusted_iron/albedo.png");
  unsigned int ironNormalMap = app.loadTexture2DBase(ASSETS_PBR_BASEPATH + "/rusted_iron/normal.png");
  unsigned int ironMetallicMap = app.loadTexture2DBase(ASSETS_PBR_BASEPATH + "/rusted_iron/metallic.png");
  unsigned int ironRoughnessMap = app.loadTexture2DBase(ASSETS_PBR_BASEPATH + "/rusted_iron/roughness.png");
  unsigned int ironAOMap = app.loadTexture2DBase(ASSETS_PBR_BASEPATH + "/rusted_iron/ao.png");

  unsigned int goldAlbedoMap = app.loadTexture2DBase(ASSETS_PBR_BASEPATH + "/gold/albedo.png");
  unsigned int goldNormalMap = app.loadTexture2DBase(ASSETS_PBR_BASEPATH + "/gold/normal.png");
  unsigned int goldMetallicMap = app.loadTexture2DBase(ASSETS_PBR_BASEPATH + "/gold/metallic.png");
  unsigned int goldRoughnessMap = app.loadTexture2DBase(ASSETS_PBR_BASEPATH + "/gold/roughness.png");
  unsigned int goldAOMap = app.loadTexture2DBase(ASSETS_PBR_BASEPATH + "/gold/ao.png");

  unsigned int grassAlbedoMap = app.loadTexture2DBase(ASSETS_PBR_BASEPATH + "/grass/albedo.png");
  unsigned int grassNormalMap = app.loadTexture2DBase(ASSETS_PBR_BASEPATH + "/grass/normal.png");
  unsigned int grassMetallicMap = app.loadTexture2DBase(ASSETS_PBR_BASEPATH + "/grass/metallic.png");
  unsigned int grassRoughnessMap = app.loadTexture2DBase(ASSETS_PBR_BASEPATH + "/grass/roughness.png");
  unsigned int grassAOMap = app.loadTexture2DBase(ASSETS_PBR_BASEPATH + "/grass/ao.png");

  unsigned int plasticAlbedoMap = app.loadTexture2DBase(ASSETS_PBR_BASEPATH + "/plastic/albedo.png");
  unsigned int plasticNormalMap = app.loadTexture2DBase(ASSETS_PBR_BASEPATH + "/plastic/normal.png");
  unsigned int plasticMetallicMap = app.loadTexture2DBase(ASSETS_PBR_BASEPATH + "/plastic/metallic.png");
  unsigned int plasticRoughnessMap = app.loadTexture2DBase(ASSETS_PBR_BASEPATH + "/plastic/roughness.png");
  unsigned int plasticAOMap = app.loadTexture2DBase(ASSETS_PBR_BASEPATH + "/plastic/ao.png");

  unsigned int wallAlbedoMap = app.loadTexture2DBase(ASSETS_PBR_BASEPATH + "/wall/albedo.png");
  unsigned int wallNormalMap = app.loadTexture2DBase(ASSETS_PBR_BASEPATH + "/wall/normal.png");
  unsigned int wallMetallicMap = app.loadTexture2DBase(ASSETS_PBR_BASEPATH + "/wall/metallic.png");
  unsigned int wallRoughnessMap = app.loadTexture2DBase(ASSETS_PBR_BASEPATH + "/wall/roughness.png");
  unsigned int wallAOMap = app.loadTexture2DBase(ASSETS_PBR_BASEPATH + "/wall/ao.png");

  app.run([=, &pbrShader, &cam, &app, &dm](float a_delta){
    cam.update(app.getWindow(), a_delta);

    pbrShader.use();{
      app.useVAO("sphere");

      // setup light
      for (unsigned int i = 0; i < sizeof(lightPositions) / sizeof(lightPositions[0]); ++i)
      {
        glm::vec3 newPos = lightPositions[i] + glm::vec3(sin(glfwGetTime() * 5.0) * 5.0, 0.0, 0.0);
        newPos = lightPositions[i];
        pbrShader.setVec3("lightPositions[" + std::to_string(i) + "]", newPos);
        pbrShader.setVec3("lightColors[" + std::to_string(i) + "]", lightColors[i]);
      }

      // rusted iron
      glActiveTexture(GL_TEXTURE4);
      glBindTexture(GL_TEXTURE_2D, ironAlbedoMap);
      glActiveTexture(GL_TEXTURE5);
      glBindTexture(GL_TEXTURE_2D, ironNormalMap);
      glActiveTexture(GL_TEXTURE6);
      glBindTexture(GL_TEXTURE_2D, ironMetallicMap);
      glActiveTexture(GL_TEXTURE7);
      glBindTexture(GL_TEXTURE_2D, ironRoughnessMap);
      glActiveTexture(GL_TEXTURE8);
      glBindTexture(GL_TEXTURE_2D, ironAOMap);
      pbrShader.setModelMatrix(glm::vec3(-5.0, 0.0, 2.0));
      glDrawElements(GL_TRIANGLE_STRIP, dm.getSphereVerticesCount(), GL_UNSIGNED_INT, 0);

      // gold
      glActiveTexture(GL_TEXTURE4);
      glBindTexture(GL_TEXTURE_2D, goldAlbedoMap);
      glActiveTexture(GL_TEXTURE5);
      glBindTexture(GL_TEXTURE_2D, goldNormalMap);
      glActiveTexture(GL_TEXTURE6);
      glBindTexture(GL_TEXTURE_2D, goldMetallicMap);
      glActiveTexture(GL_TEXTURE7);
      glBindTexture(GL_TEXTURE_2D, goldRoughnessMap);
      glActiveTexture(GL_TEXTURE8);
      glBindTexture(GL_TEXTURE_2D, goldAOMap);
      pbrShader.setModelMatrix(glm::vec3(-3.0, 0.0, 2.0));
      glDrawElements(GL_TRIANGLE_STRIP, dm.getSphereVerticesCount(), GL_UNSIGNED_INT, 0);

      // grass
      glActiveTexture(GL_TEXTURE4);
      glBindTexture(GL_TEXTURE_2D, grassAlbedoMap);
      glActiveTexture(GL_TEXTURE5);
      glBindTexture(GL_TEXTURE_2D, grassNormalMap);
      glActiveTexture(GL_TEXTURE6);
      glBindTexture(GL_TEXTURE_2D, grassMetallicMap);
      glActiveTexture(GL_TEXTURE7);
      glBindTexture(GL_TEXTURE_2D, grassRoughnessMap);
      glActiveTexture(GL_TEXTURE8);
      glBindTexture(GL_TEXTURE_2D, grassAOMap);
      pbrShader.setModelMatrix(glm::vec3(-1.0, 0.0, 2.0));
      glDrawElements(GL_TRIANGLE_STRIP, dm.getSphereVerticesCount(), GL_UNSIGNED_INT, 0);

      // plastic
      glActiveTexture(GL_TEXTURE4);
      glBindTexture(GL_TEXTURE_2D, plasticAlbedoMap);
      glActiveTexture(GL_TEXTURE5);
      glBindTexture(GL_TEXTURE_2D, plasticNormalMap);
      glActiveTexture(GL_TEXTURE6);
      glBindTexture(GL_TEXTURE_2D, plasticMetallicMap);
      glActiveTexture(GL_TEXTURE7);
      glBindTexture(GL_TEXTURE_2D, plasticRoughnessMap);
      glActiveTexture(GL_TEXTURE8);
      glBindTexture(GL_TEXTURE_2D, plasticAOMap);
      pbrShader.setModelMatrix(glm::vec3(1.0, 0.0, 2.0));
      glDrawElements(GL_TRIANGLE_STRIP, dm.getSphereVerticesCount(), GL_UNSIGNED_INT, 0);

      // wall
      glActiveTexture(GL_TEXTURE4);
      glBindTexture(GL_TEXTURE_2D, wallAlbedoMap);
      glActiveTexture(GL_TEXTURE5);
      glBindTexture(GL_TEXTURE_2D, wallNormalMap);
      glActiveTexture(GL_TEXTURE6);
      glBindTexture(GL_TEXTURE_2D, wallMetallicMap);
      glActiveTexture(GL_TEXTURE7);
      glBindTexture(GL_TEXTURE_2D, wallRoughnessMap);
      glActiveTexture(GL_TEXTURE8);
      glBindTexture(GL_TEXTURE_2D, wallAOMap);
      pbrShader.setModelMatrix(glm::vec3(3.0, 0.0, 2.0));
      glDrawElements(GL_TRIANGLE_STRIP, dm.getSphereVerticesCount(), GL_UNSIGNED_INT, 0);
    }
  });
}