#include "tinygl/glApp.h"
#include "tinygl/camera.h"
#include "tinygl/shader_s.h"
#include "tinygl/model_loader.h"

glm::vec3 lampPosVec[] = {
  glm::vec3(2.f, 2.f, 1.f),
  glm::vec3(2.f, -2.f, -1.f),
  glm::vec3(-2.f, 2.f, 1.f),
  glm::vec3(-2.f, -2.f, -1.f)
};

// user define vertex structure, because only user know the input model vertex data format
struct Vertex
{
  glm::vec3 Position;
  glm::vec3 Normal;
  glm::vec2 TexCoords;
  glm::vec3 Tangent;
  glm::vec3 Bitangent;
};

// user defined load texture type
std::vector<aiTextureType> nanosuitTextureTypes = {aiTextureType_DIFFUSE, aiTextureType_SPECULAR, aiTextureType_HEIGHT};

int main()
{
  GLApp app;
  app.init();
  app.setProjectionMatrix(45.f, 0.1f, 100.f);
  Camera cam;
  cam.addViewMatrixUpdateListener(&app);

  auto modelLoader = ModelLoader<Vertex>(3);
  modelLoader.setProcessMeshHandler(STANDARD_OBJ_LOADER(Vertex, nanosuitTextureTypes));

  modelLoader.load(ASSETS_3D_BASEPATH + "nanosuit/nanosuit.obj", "nanosuit");

  Shader modelShader(COMMON_SHADERS_BASEPATH + "/dnsShader.vs", COMMON_SHADERS_BASEPATH + "/dnsShader.fs");
  modelShader.init("model", "view", &cam, "projection", app.getProjectionMatrix());
  modelShader.setModelMatrix(glm::vec3(0.0f, -2.0f, 0.0f), glm::vec3(.2f, .2f, .2f));
  modelShader.setTextureBaseName(aiTextureType_DIFFUSE, "texture_diffuse");
  modelShader.setTextureBaseName(aiTextureType_SPECULAR, "texture_specular");
  modelShader.setTextureBaseName(aiTextureType_HEIGHT, "texture_normal");

  modelShader.setFloat("texture_shinness", 256.0f);
  modelShader.setVec3("directlight.direction", {-0.2f, -1.0f, -0.3f});
  modelShader.setVec3("directlight.ambient", {0.05f, 0.05f, 0.05f});
  modelShader.setVec3("directlight.diffuse", {0.3f, 0.3f, 0.3f});
  modelShader.setVec3("directlight.specular", {0.2f, 0.2f, 0.2f});

  for(int i = 0; i < 4; ++i){
    modelShader.setVec3("pointlights[" + std::to_string(i) + "].ambient",  {.1f, .1f, .1f});
    modelShader.setVec3("pointlights[" + std::to_string(i) + "].diffuse",  {.5f, 0.5f, 0.5f});
    modelShader.setVec3("pointlights[" + std::to_string(i) + "].specular", {1.f, 1.f, 1.f});
    modelShader.setFloat("pointlights[" + std::to_string(i) + "].constant",  1.0f);
    modelShader.setFloat("pointlights[" + std::to_string(i) + "].linear",    0.05f);
    modelShader.setFloat("pointlights[" + std::to_string(i) + "].quadratic", 0.015f);
    modelShader.setVec3("pointlights[" + std::to_string(i) + "].position", lampPosVec[i]);
  }

  modelShader.setVec3("spotlight.position", cam.GetCameraPos());
  modelShader.setVec3("spotlight.direction", cam.GetCameraFront());
  modelShader.setFloat("spotlight.innerCutoff", glm::cos(glm::radians(12.5f)));
  modelShader.setFloat("spotlight.outerCutoff", glm::cos(glm::radians(17.5f)));

  app.run([=, &modelShader, &modelLoader, &cam, &app](float a_delta){
    cam.update(app.getWindow(), a_delta);
    modelShader.use();
    modelShader.setVec3("cameraPos", cam.GetCameraPos());
    modelShader.setVec3("spotlight.position", cam.GetCameraPos());
    modelShader.setVec3("spotlight.direction", cam.GetCameraFront());
    modelLoader.Draw({"nanosuit"}, modelShader);
  });
}