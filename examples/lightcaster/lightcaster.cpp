#include "tinygl/glApp.h"
#include "tinygl/camera.h"
#include "tinygl/shader_s.h"

int main()
{
  // ----------------------------------------------------------
  float vertices[] = {
    // vertex           // texture   // normal
    0.5f, -0.5f, -0.5f,  1.0f, 0.0f, 0.0f,  0.0f, -1.0f,
    0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 0.0f,  0.0f, -1.0f,
    -0.5f,  0.5f, -0.5f,  0.0f, 1.0f, 0.0f,  0.0f, -1.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, 0.0f, 0.0f,  0.0f, -1.0f,

    0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 0.0f,  0.0f, 1.0f,
    0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 0.0f,  0.0f, 1.0f,
    -0.5f,  0.5f,  0.5f,  0.0f, 1.0f, 0.0f,  0.0f, 1.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f, 0.0f,  0.0f, 1.0f,

    -0.5f,  0.5f, -0.5f,  1.0f, 1.0f, -1.0f,  0.0f,  0.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, 1.0f, -1.0f,  0.0f,  0.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f, -1.0f,  0.0f,  0.0f,
    -0.5f,  0.5f,  0.5f,  1.0f, 0.0f, -1.0f,  0.0f,  0.0f,

    0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 1.0f,  0.0f,  0.0f,
    0.5f, -0.5f, -0.5f,  0.0f, 1.0f, 1.0f,  0.0f,  0.0f,
    0.5f, -0.5f,  0.5f,  0.0f, 0.0f, 1.0f,  0.0f,  0.0f,
    0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 1.0f,  0.0f,  0.0f,

    0.5f, -0.5f, -0.5f,  1.0f, 1.0f, 0.0f, -1.0f,  0.0f,
    0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 0.0f, -1.0f,  0.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f, 0.0f, -1.0f,  0.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, 1.0f, 0.0f, -1.0f,  0.0f,

    0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 0.0f,  1.0f,  0.0f,
    0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 0.0f,  1.0f,  0.0f,
    -0.5f,  0.5f,  0.5f,  0.0f, 0.0f, 0.0f,  1.0f,  0.0f,
    -0.5f,  0.5f, -0.5f,  0.0f, 1.0f, 0.0f,  1.0f,  0.0f
  };
  int vertexCnt = 36;
  unsigned int indices[vertexCnt];
  for(int i = 0; i < 6; ++i){
    indices[i*6 + 0] = 0 + 4 * i;
    indices[i*6 + 1] = 1 + 4 * i;
    indices[i*6 + 2] = 3 + 4 * i;
    indices[i*6 + 3] = 1 + 4 * i;
    indices[i*6 + 4] = 2 + 4 * i;
    indices[i*6 + 5] = 3 + 4 * i;
  }
  int cudeNum = 10;
  glm::vec3 cubePositions[] = {
    glm::vec3( 0.0f,  0.0f,  0.0f),
    glm::vec3( 2.0f,  5.0f, -15.0f),
    glm::vec3(-1.5f, -2.2f, -2.5f),
    glm::vec3(-3.8f, -2.0f, -12.3f),
    glm::vec3( 2.4f, -0.4f, -3.5f),
    glm::vec3(-1.7f,  3.0f, -7.5f),
    glm::vec3( 1.3f, -2.0f, -2.5f),
    glm::vec3( 1.5f,  2.0f, -2.5f),
    glm::vec3( 1.5f,  0.2f, -1.5f),
    glm::vec3(-1.3f,  1.0f, -1.5f)
  };
  glm::vec3 axis[10];
  for(int i=0;i<cudeNum;++i){
    axis[i] = {static_cast<float>(rand()) / static_cast<float>(RAND_MAX), static_cast<float>(rand()) / static_cast<float>(RAND_MAX), static_cast<float>(rand()) / static_cast<float>(RAND_MAX)};
  }
  // ----------------------------------------------------------

  GLApp app;
  app.init();
  app.setProjectionMatrix(45.f, 0.1f, 100.f);
  Camera cam;
  cam.addViewMatrixUpdateListener(&app);

  app.loadTexture2D(ASSETS_2D_BASEPATH + "/container2.png", 0);
  app.loadTexture2D(ASSETS_2D_BASEPATH + "/container2_specular.png", 1);
  app.loadTexture2D(ASSETS_2D_BASEPATH + "/matrix.jpg", 2);

  glm::vec3 lampPos(1.f, 1.f, 1.f);
  glm::vec3 lampRotateAxis(0.f, 0.f, lampPos.z);

  Shader objShader(getExecutablePath() + "/objshader.vs", getExecutablePath() + "/objshader.fs");
  objShader.use();
  objShader.setVec3("cameraPos", cam.GetCameraPos());

  glm::mat4 model = glm::mat4(1.0f);
  objShader.setMatrix4fv("model", model);
  objShader.setMatrix4fv("view", cam.GetViewMatrix());
  objShader.setMatrix4fv("projection", app.getProjectionMatrix());

  objShader.setVec3("light.position", lampPos);
  objShader.setVec3("light.ambient",  {.1f, .1f, .1f});
  objShader.setVec3("light.diffuse",  {.5f, 0.5f, 0.5f});
  objShader.setVec3("light.specular", {1.f, 1.f, 1.f});
  objShader.setFloat("light.constant",  1.0f);
  objShader.setFloat("light.linear",    0.05f);
  objShader.setFloat("light.quadratic", 0.015f);

  objShader.setVec3("spotlight.position", cam.GetCameraPos());
  objShader.setVec3("spotlight.direction", cam.GetCameraFront());
  objShader.setFloat("spotlight.innerCutoff", glm::cos(glm::radians(12.5f)));
  objShader.setFloat("spotlight.outerCutoff", glm::cos(glm::radians(17.5f)));

  objShader.setVec3("material.ambient",  {1.0f, 0.5f, 0.31f});
  objShader.setInt("material.diffuse",  0);
  objShader.setInt("material.specular", 1);
  objShader.setInt("material.emission", 2);
  objShader.setFloat("material.shinness", 32.0f);

  Shader lampShader(getExecutablePath() + "/lampshader.vs", getExecutablePath() + "/lampshader.fs");
  lampShader.use();
  model = glm::mat4(1.0f);
  model = glm::translate(model, lampPos);
  model = glm::scale(model, glm::vec3(0.2f));
  lampShader.setMatrix4fv("model", model);
  lampShader.setMatrix4fv("view", cam.GetViewMatrix());
  lampShader.setMatrix4fv("projection", app.getProjectionMatrix());

  app.generateNewVAOFromArray("objCube", vertices, sizeof(vertices), {{0, 3}, {2, 2}, {3, 3}}, indices, sizeof(indices));
  app.generateNewVAOFromArray("lampCube", "objCube", {{0, 0}});

  app.run([=, &objShader, &lampShader, &cam, &app](float a_delta){
    cam.update(app.getWindow(), a_delta);

    lampShader.use();
    app.useVAO("lampCube");
    glm::mat4 model = glm::mat4(1.0f);
    glm::vec3 rotateLampPos = glm::rotate(lampPos-lampRotateAxis, (float)glfwGetTime(), glm::vec3(0, 0, 1)) + lampRotateAxis;
    model = glm::translate(model, rotateLampPos);
    model = glm::scale(model, glm::vec3(0.2f));
    model = glm::rotate(model, (float)glfwGetTime(), glm::vec3(1, 1, 1));
    lampShader.setMatrix4fv("model", model);
    lampShader.setMatrix4fv("view", cam.GetViewMatrix());
    glDrawElements(GL_TRIANGLES, vertexCnt, GL_UNSIGNED_INT, 0);

    objShader.use();
    app.useVAO("objCube");
    objShader.setVec3("light.position", rotateLampPos);
    objShader.setVec3("cameraPos", cam.GetCameraPos());
    objShader.setVec3("spotlight.position", cam.GetCameraPos());
    objShader.setVec3("spotlight.direction", cam.GetCameraFront());

    for(int i = 0; i < cudeNum; i++) {
      glm::mat4 model = glm::mat4(1.0f);
      model = glm::translate(model, cubePositions[i]);
      model = glm::rotate(model, (float)glfwGetTime(), axis[i]);
      objShader.setMatrix4fv("model", model);
      objShader.setMatrix4fv("view", cam.GetViewMatrix());
      glDrawElements(GL_TRIANGLES, vertexCnt, GL_UNSIGNED_INT, 0);
    }
  });
}