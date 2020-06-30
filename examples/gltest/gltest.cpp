#include "tinygl/glApp.h"
#include "tinygl/camera.h"
#include "tinygl/shader_s.h"

int main()
{
  GLApp app;
  app.init();
  app.setProjectionMatrix(45.f, 0.1f, 100.f);
  Camera cam;
  cam.addViewMatrixUpdateListener(&app);

  app.loadTexture2D(ASSETS_2D_BASEPATH + "/container.jpg", 0);
  app.loadTexture2D(ASSETS_2D_BASEPATH + "/awesomeface.png", 1);

  glm::vec3 lampPos(3.f, 2.f, -6.f);
  glm::vec3 lampRotateAxis(2.f, lampPos.y, -5.f);

  Shader objShader(getExecutablePath() + "/color.vs", getExecutablePath() + "/color.fs");
  objShader.use();
  objShader.setInt("texture1", 0);
  objShader.setInt("texture2", 1);
  objShader.setVec3("objectColor", {1.f, 0.5f, 0.31f});
  objShader.setVec3("lightColor", {1.f, 1.f, 1.f});
  objShader.setVec3("lampPos", lampPos);
  objShader.setVec3("cameraPos", cam.GetCameraPos());
  glm::mat4 model = glm::mat4(1.0f);
  objShader.setMatrix4fv("model", model);
  objShader.setMatrix4fv("view", cam.GetViewMatrix());
  objShader.setMatrix4fv("projection", app.getProjectionMatrix());

  Shader lampShader(getExecutablePath() + "/lampshader.vs", getExecutablePath() + "/lampshader.fs");
  lampShader.use();
  model = glm::mat4(1.0f);
  model = glm::translate(model, lampPos);
  model = glm::scale(model, glm::vec3(0.2f));
  lampShader.setMatrix4fv("model", model);
  lampShader.setMatrix4fv("view", cam.GetViewMatrix());
  lampShader.setMatrix4fv("projection", app.getProjectionMatrix());

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
  unsigned int indices[36];
  for(int i = 0; i < 6; ++i){
    indices[i*6 + 0] = 0 + 4 * i;
    indices[i*6 + 1] = 1 + 4 * i;
    indices[i*6 + 2] = 3 + 4 * i;
    indices[i*6 + 3] = 1 + 4 * i;
    indices[i*6 + 4] = 2 + 4 * i;
    indices[i*6 + 5] = 3 + 4 * i;
  }
  int vertexCnt = 36;

  glm::vec3 cubePositions[] = {
    glm::vec3( 2.0f,  -2.0f,  -5.0f),
    glm::vec3( -2.0f,  2.0f,  -5.0f),
    glm::vec3( -2.0f,  -2.0f,  -5.0f),
  };

  glm::vec3 axis[3];
  for(int i=0;i<3;++i){
    axis[i] = {static_cast<float>(rand()) / static_cast<float>(RAND_MAX), static_cast<float>(rand()) / static_cast<float>(RAND_MAX), static_cast<float>(rand()) / static_cast<float>(RAND_MAX)};
  }

  app.generateNewVAOFromArray("objCube", vertices, sizeof(vertices), {{0, 3}, {2, 2}, {3, 3}}, indices, sizeof(indices));
  app.generateNewVAOFromArray("lampCube", "objCube", {{0, 0}});

  app.run([=, &objShader, &lampShader, &cam, &app](float a_delta){
    cam.update(app.getWindow(), a_delta);

    lampShader.use();
    app.useVAO("lampCube");
    glm::mat4 model = glm::mat4(1.0f);
    glm::vec3 rotateLampPos = glm::rotate(lampPos-lampRotateAxis, (float)glfwGetTime(), glm::vec3(0, 1, 0)) + lampRotateAxis;
    model = glm::translate(model, rotateLampPos);
    model = glm::scale(model, glm::vec3(0.2f));
    model = glm::rotate(model, (float)glfwGetTime(), glm::vec3(1, 1, 1));
    lampShader.setMatrix4fv("model", model);
    lampShader.setMatrix4fv("view", cam.GetViewMatrix());
    glDrawElements(GL_TRIANGLES, vertexCnt, GL_UNSIGNED_INT, 0);

    objShader.use();
    app.useVAO("objCube");
    objShader.setVec3("lampPos", rotateLampPos);
    objShader.setVec3("cameraPos", cam.GetCameraPos());

    for(int i = 0; i < 3; i++) {
      glm::mat4 model = glm::mat4(1.0f);
      model = glm::translate(model, cubePositions[i]);
      model = glm::rotate(model, (float)glfwGetTime(), axis[i]);
      objShader.setMatrix4fv("model", model);
      objShader.setMatrix4fv("view", cam.GetViewMatrix());
      glDrawElements(GL_TRIANGLES, vertexCnt, GL_UNSIGNED_INT, 0);
    }
  });
}