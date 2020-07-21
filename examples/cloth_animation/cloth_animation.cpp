#include "tinygl/glApp.h"
#include "tinygl/default_mesh.h"
#include "tinygl/camera.h"
#include "tinygl/shader_s.h"
#include "tinygl/light.h"
#include "tinygl/cloth_model.h"

void render(Shader&, DefaultMesh&, Cloth&, GLApp&);
int main() {
  GLApp app;
  app.init();
  app.setProjectionMatrix(45.f, 0.1f, 100.f);
  app.useSkyBox("sor_sea", 3);
  Camera cam;
  cam.addViewMatrixUpdateListener(&app);
  DefaultMesh dm(&app, &cam);
  dm.initPlaneData(); // ground
  dm.initSphere(); // lamp
  Light light;

  // floor resource
  app.loadTexture2D(ASSETS_2D_BASEPATH + "/wood.png", 1);
  app.generateNewVAOFromArray("floor", dm.getPlaneVerticesArray(), dm.getPlaneVerticesArraySize(), {{0, 3}, {1, 2}, {2, 3}}, dm.getPlaneIndicesArray(), dm.getPlaneIndicesArraySize());

  // cloth resource
  app.loadTexture2D(ASSETS_2D_BASEPATH + "/awesomeface.png", 2);
  Cloth cloth;
  cloth.init(10, 10, 25, 1);
  auto clothVBO = cloth.getUpdatedVBO();
  auto clothEBO = cloth.getEBO();
  app.generateNewVAOFromArray("cloth", clothVBO.data(), clothVBO.size() * sizeof(float),
                        {{0, 3}, {1, 2}, {2, 3}}, clothEBO.data(), clothEBO.size() * sizeof(uint32_t));

  // fixed lamp
  const glm::vec3 LampPosition = {-1, 2, 1};

  // scene shader
  // this shader is only used for second pass
  // render both floor and cloth
  Shader sceneShader(getExecutablePath() + "/scene.vs", getExecutablePath() + "/scene.fs");
  sceneShader.init("model", "view", &cam, "projection", app.getProjectionMatrix(), "cameraPos");

  sceneShader.setVec3("directLight.direction", {-0.2f, -1.0f, -0.3f});
  sceneShader.setVec3("directLight.ambient", {0.05f, 0.05f, 0.05f});
  sceneShader.setVec3("directLight.diffuse", {0.2f, 0.2f, 0.2f});
  sceneShader.setVec3("directLight.specular", {0.2f, 0.2f, 0.2f});

  sceneShader.setVec3("pointlight.position",  LampPosition);
  sceneShader.setVec3("pointlight.ambient",  {.5f, .5f, .5f});
  sceneShader.setVec3("pointlight.diffuse",  {.5f, 0.5f, 0.5f});
  sceneShader.setVec3("pointlight.specular", {1.f, 1.f, 1.f});
  sceneShader.setFloat("pointlight.constant",  1.0f);
  sceneShader.setFloat("pointlight.linear",    0.05f);
  sceneShader.setFloat("pointlight.quadratic", 0.015f);

  sceneShader.setFloat("material.shinness", 32.0f);
  sceneShader.setVec3("material.specular", {0.15, 0.15, 0.15});

  const GLuint shadowMapChannel = 0;
  sceneShader.setInt("shadowMap", shadowMapChannel);

  // set up light
  ShadowMeta meta;
  meta.lookat = {0, 0, 0};
  meta.up = {0, 1, 0};
  meta.shadowWidth = 1024;
  meta.shadowHeight = 1024;
  meta.fov = 90.f;
  meta.nearPlane = 1;
  meta.farPlane = 25;
  light.enableShadow(meta);
  light.setPosition(LampPosition);

  app.run([=, &sceneShader, &cam, &app, &dm, &light, &cloth](float a_delta){
    cam.update(app.getWindow(), a_delta);

    // update the cloth
    bool dirty = cloth.update(a_delta);
    if(dirty) {
      auto clothUpdatedVBO = cloth.getUpdatedVBO();
      app.updateVBOData("cloth", clothUpdatedVBO.data(), clothUpdatedVBO.size() * sizeof(float));
    }

    // first pass, light perspective
    light.depthPass(std::bind(render, std::placeholders::_1, std::ref(dm), std::ref(cloth), std::ref(app)));

    app.resetViewport();
    // second pass
    // render lamp using default shader
    dm.drawSphere(glm::vec4(1, 1, 1, 0.8), LampPosition, glm::vec3(0.1));
    // render scene
    sceneShader.use(); {
      sceneShader.setMatrix4fv("lightSpaceMatrix", light.getLightSpaceMat());
      light.activateDepthMap(shadowMapChannel);
      render(sceneShader, dm, cloth, app);
    }
  });
}

// called in both first and second pass
void render(Shader& shader, DefaultMesh& dm, Cloth& cloth, GLApp& app)
{
  // render everything except the lamp
  shader.use();

  // render floor
  app.useVAO("floor");
  shader.setModelMatrix(glm::vec3(0, -0.2, 0), glm::vec3(1), 0, glm::vec3());
  shader.setInt("material.diffuseTexture", 1);
  glDrawElements(GL_TRIANGLES, dm.getPlaneVerticesCount(), GL_UNSIGNED_INT, 0);

  // render cloth
  app.useVAO("cloth");
  shader.setModelMatrix(glm::vec3(0, 0.5, 0), glm::vec3(0.005), 0, glm::vec3());
  shader.setInt("material.diffuseTexture", 2);
  glDrawElements(GL_TRIANGLES, cloth.getIndicesCount(), GL_UNSIGNED_INT, 0);
}