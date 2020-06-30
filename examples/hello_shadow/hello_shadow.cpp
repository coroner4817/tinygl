#include "tinygl/glApp.h"
#include "tinygl/default_mesh.h"
#include "tinygl/camera.h"
#include "tinygl/shader_s.h"
#include "tinygl/model_loader.h"
#include "tinygl/light.h"

// user define vertex structure, because only user know the input model vertex data format
struct Vertex
{
  glm::vec3 Position;
  glm::vec3 Normal;
  glm::vec2 TexCoords;
  glm::vec3 Tangent;
  glm::vec3 Bitangent;
};

#define F16
#ifdef BUNNY
// user defined load texture type
std::vector<aiTextureType> TextureTypes = {};
const std::string MODEL_NAME = "bunny";
#endif
#ifdef F16
std::vector<aiTextureType> TextureTypes = {aiTextureType_DIFFUSE};
const std::string MODEL_NAME = "f16";
#endif
#ifdef NANOSUIT
std::vector<aiTextureType> TextureTypes = {aiTextureType_DIFFUSE, aiTextureType_SPECULAR, aiTextureType_HEIGHT};
const std::string MODEL_NAME = "nanosuit";
#endif

void render(Shader& shader, ModelLoader<Vertex>& loader, DefaultMesh& dm,  GLApp& app, const glm::vec3& lamppos);

int main()
{
  GLApp app;
  app.init();
  app.setProjectionMatrix(45.f, 0.1f, 100.f);
  app.useSkyBox("sor_sea", 3);
  Camera cam;
  cam.addViewMatrixUpdateListener(&app);
  DefaultMesh dm(&app, &cam);
  dm.setObjAttrComp(true);
  dm.initPlaneData();
  dm.initSphere();
  Light light;

  app.loadTexture2D(ASSETS_2D_BASEPATH + "/wood.png", 1);
  app.loadTexture2D(ASSETS_2D_BASEPATH + "/matrix.jpg", 2);

  auto modelLoader = ModelLoader<Vertex>(4);
  modelLoader.setProcessMeshHandler(STANDARD_OBJ_LOADER(Vertex, TextureTypes));
  modelLoader.load(ASSETS_3D_BASEPATH + MODEL_NAME + "/" + MODEL_NAME + ".obj", MODEL_NAME);

  app.generateNewVAOFromArray("plane", dm.getPlaneVerticesArray(), dm.getPlaneVerticesArraySize(), {{0, 3}, {1, 3}, {2, 2}}, dm.getPlaneIndicesArray(), dm.getPlaneIndicesArraySize());

  Shader shadowShader(getExecutablePath() + "/shadow.vs", getExecutablePath() + "/shadow.fs");
  shadowShader.init("model", "view", &cam, "projection", app.getProjectionMatrix(), "cameraPos");

  shadowShader.setInt("shadowMap", 0);
  shadowShader.setFloat("texture_shinness", 256.0f);
  shadowShader.setVec3("myTexture.specular", {0.15, 0.15, 0.15});

  shadowShader.setVec3("pointlight.ambient",  {.1f, .1f, .1f});
  shadowShader.setVec3("pointlight.diffuse",  {.5f, 0.5f, 0.5f});
  shadowShader.setVec3("pointlight.specular", {1.f, 1.f, 1.f});
  shadowShader.setFloat("pointlight.constant",  1.0f);
  shadowShader.setFloat("pointlight.linear",    0.05f);
  shadowShader.setFloat("pointlight.quadratic", 0.015f);

  shadowShader.setTextureBaseName(aiTextureType_DIFFUSE, "texture_diffuse");
  shadowShader.setTextureBaseName(aiTextureType_SPECULAR, "texture_specular");
  shadowShader.setTextureBaseName(aiTextureType_HEIGHT, "texture_normal");

  glm::vec3 lampPos = {1, 1, 1};
  glm::vec3 lampAxis = {0, 1, 0};
  ShadowMeta meta;
  meta.lookat = {0, 0, 0};
  meta.up = {0, 1, 0};
  meta.shadowWidth = 1024;
  meta.shadowHeight = 1024;
  meta.fov = 90.f;
  meta.nearPlane = 1;
  meta.farPlane = 25;
  light.enableShadow(meta);

  app.run([=, &shadowShader, &modelLoader, &cam, &app, &dm, &light](float a_delta){
    cam.update(app.getWindow(), a_delta);

    glm::vec3 lampPosRotate = glm::rotate(lampPos - lampPos * lampAxis, (float)glfwGetTime(), lampAxis) + lampPos * lampAxis;
    light.setPosition(lampPosRotate);

    // first depth pass
    light.depthPass(std::bind(render, std::placeholders::_1, std::ref(modelLoader), std::ref(dm), std::ref(app), std::ref(lampPosRotate)));

    // second pass
    app.resetViewport();
    shadowShader.use();
    shadowShader.setMatrix4fv("lightSpaceMatrix", light.getLightSpaceMat());
    shadowShader.setVec3("lightPos", light.getPosition());
    light.activateDepthMap(0); // we set the shadowMap to channel 0 before
    render(shadowShader, modelLoader, dm, app, lampPosRotate);
  });
}

void render(Shader& shader, ModelLoader<Vertex>& loader, DefaultMesh& dm, GLApp& app, const glm::vec3& lamppos)
{
  shader.setInt("hasTexture", 0);
  dm.drawSphere(glm::vec4(1, 1, 1, 0.8), lamppos, glm::vec3(0.1));

  shader.use();
  shader.setVec3("pointlight.position", lamppos);
  app.useVAO("plane");
  shader.setModelMatrix(glm::vec3(0, -0.1, 0), glm::vec3(1), 0, glm::vec3());
  shader.setInt("myTexture.diffuseTexture", 1);
  glDrawElements(GL_TRIANGLES, dm.getPlaneVerticesCount(), GL_UNSIGNED_INT, 0);

#ifdef BUNNY
  shader.setInt("myTexture.diffuseTexture", 1);
#else
  shader.setInt("hasTexture", 1);
#endif
  shader.setModelMatrix(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(.2f, .2f, .2f));
  loader.Draw({MODEL_NAME}, shader);
}