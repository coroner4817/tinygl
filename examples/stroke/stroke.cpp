#include "tinygl/glApp.h"
#include "tinygl/camera.h"
#include "tinygl/shader_s.h"
#include "tinygl/stroke.h"
#include "stroke_data.h"

void initTestStroke(Stroke* pStroke_, glm::mat4 tf)
{
  float x_b =  - 0.8f, x_e = -x_b;
  float step = 0.01f;
  for(float x = x_b; x < x_e; x += step){
    float scl = x / x_e * 8 * glm::pi<float>();
    glm::vec3 p = glm::vec3(tf * glm::vec4(x, glm::sin(scl), glm::cos(scl), 1.0f));
    pStroke_->addPoint({p, 0.1});
  }
  pStroke_->addLineBreak();
}

void addTestStroke(Stroke* pStroke_, std::vector<glm::vec3> vec)
{
  for(const auto& i : vec){
    pStroke_->addPoint({i * 5, 0.1});
  }
  pStroke_->addLineBreak();
}

int main()
{
  std::vector<glm::vec3> lampPosVec = {
    glm::vec3(2.f, 2.f, 1.f),
    glm::vec3(2.f, -2.f, -1.f),
    glm::vec3(-2.f, 2.f, 1.f),
    glm::vec3(-2.f, -2.f, -1.f)
  };

  GLApp app;
  app.init();
  app.setProjectionMatrix(45.f, 0.1f, 100.f);
  app.useSkyBox("sor_sea", 0);
  Camera cam;
  cam.addViewMatrixUpdateListener(&app);

  Stroke stroke1;
  stroke1.setSides(5);
  stroke1.enableKalman(false);
  // initTestStroke(&stroke1, glm::mat4(1));
  addTestStroke(&stroke1, test_stroke1);
  addTestStroke(&stroke1, test_stroke2);

  const std::string textureName = "Abstract_006";
  stroke1.loadTexture2D(ASSETS_BASEPATH + "stroke/" + textureName + "/" + textureName + "_COLOR.jpg", 0);
  stroke1.loadTexture2D(ASSETS_BASEPATH + "stroke/" + textureName + "/" + textureName + "_NORM.jpg", 1);
  stroke1.loadTexture2D(ASSETS_BASEPATH + "stroke/" + textureName + "/" + textureName + "_SPEC.jpg", 2);

  Stroke stroke2;
  stroke2.setSides(5);
  stroke2.enableKalman(true);
  stroke2.setExtrudeLength(0.05);
  // initTestStroke(&stroke2, glm::mat4(1));
  addTestStroke(&stroke2, test_stroke1);
  addTestStroke(&stroke2, test_stroke2);
  stroke2.loadTexture2D(ASSETS_BASEPATH + "stroke/Ring_001/Ring_001.png", 0);

  Shader strokeShader(COMMON_SHADERS_BASEPATH + "stroke.vert", COMMON_SHADERS_BASEPATH + "stroke.frag");
  strokeShader.use();
  strokeShader.setVec3("cameraPos", cam.GetCameraPos());
  glm::mat4 model = glm::mat4(1.0f);
  strokeShader.setMatrix4fv("model", model);
  strokeShader.setMatrix4fv("view", cam.GetViewMatrix());
  strokeShader.setMatrix4fv("projection", app.getProjectionMatrix());

  strokeShader.setVec3("directlight.direction", {-0.2f, -1.0f, -0.3f});
  strokeShader.setVec3("directlight.ambient", {0.05f, 0.05f, 0.05f});
  strokeShader.setVec3("directlight.diffuse", {0.3f, 0.3f, 0.3f});
  strokeShader.setVec3("directlight.specular", {0.2f, 0.2f, 0.2f});
  for(int i = 0; i < 4; ++i){
    strokeShader.setVec3("pointlights[" + std::to_string(i) + "].ambient",  {.1f, .1f, .1f});
    strokeShader.setVec3("pointlights[" + std::to_string(i) + "].diffuse",  {.5f, 0.5f, 0.5f});
    strokeShader.setVec3("pointlights[" + std::to_string(i) + "].specular", {1.f, 1.f, 1.f});
    strokeShader.setFloat("pointlights[" + std::to_string(i) + "].constant",  1.0f);
    strokeShader.setFloat("pointlights[" + std::to_string(i) + "].linear",    0.05f);
    strokeShader.setFloat("pointlights[" + std::to_string(i) + "].quadratic", 0.015f);
    strokeShader.setVec3("pointlights[" + std::to_string(i) + "].position", lampPosVec[i]);
  }

  strokeShader.setInt("texture_diffuse0",  0);
  strokeShader.setInt("texture_normal0", 1);
  strokeShader.setInt("texture_specular0", 2);
  strokeShader.setFloat("texture_shinness", 8.0f);

  app.run([=, &strokeShader, &cam, &app, &stroke1, &stroke2](float a_delta){
    cam.update(app.getWindow(), a_delta);

    strokeShader.use();
    stroke1.use();
    strokeShader.setMatrix4fv("view", cam.GetViewMatrix());
    strokeShader.setVec3("cameraPos", cam.GetCameraPos());
    glDrawElements(GL_TRIANGLES, stroke1.getVerticesCount(), GL_UNSIGNED_INT, 0);

    strokeShader.use();
    stroke2.use();
    strokeShader.setMatrix4fv("view", cam.GetViewMatrix());
    strokeShader.setVec3("cameraPos", cam.GetCameraPos());
    glDrawElements(GL_TRIANGLES, stroke2.getVerticesCount(), GL_UNSIGNED_INT, 0);
  });
}