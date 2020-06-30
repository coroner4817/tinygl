#include "tinygl/glApp.h"
#include "tinygl/camera.h"
#include "tinygl/shader_s.h"
#include "tinygl/default_mesh.h"

int main()
{
  // ----------------------------------------------------------
  // user define data
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
  int lampNum = 4;
  glm::vec3 lampPosVec[] = {
    glm::vec3(2.f, 2.f, 1.f),
    glm::vec3(2.f, -2.f, -1.f),
    glm::vec3(-2.f, 2.f, 1.f),
    glm::vec3(-2.f, -2.f, -1.f)
  };
  glm::vec3 lampRotAxisVec[] = {
    glm::vec3(0, 0, 1),
    glm::vec3(0, 1, 0),
    glm::vec3(1, 0, 0),
    glm::vec3(0, -1, 0)
  };
  // ----------------------------------------------------------

  GLApp app;
  app.init();
  app.setProjectionMatrix(45.f, 0.1f, 100.f);
  Camera cam;
  cam.addViewMatrixUpdateListener(&app);
  DefaultMesh dm;
  dm.initCubeData();
  dm.initSphereData(1, 15, 15);

  app.loadTexture2D(ASSETS_2D_BASEPATH + "/container2.png", 0);
  app.loadTexture2D(ASSETS_2D_BASEPATH + "/container2_specular.png", 1);
  app.loadTexture2D(ASSETS_2D_BASEPATH + "/matrix.jpg", 2);

  Shader objShader(getExecutablePath() + "/objshader.vs", getExecutablePath() + "/objshader.fs");
  objShader.use();
  objShader.setVec3("cameraPos", cam.GetCameraPos());

  glm::mat4 model = glm::mat4(1.0f);
  objShader.setMatrix4fv("model", model);
  objShader.setMatrix4fv("view", cam.GetViewMatrix());
  objShader.setMatrix4fv("projection", app.getProjectionMatrix());

  // direct light is fixed
  objShader.setVec3("directlight.direction", {-0.2f, -1.0f, -0.3f});
  objShader.setVec3("directlight.ambient", {0.05f, 0.05f, 0.05f});
  objShader.setVec3("directlight.diffuse", {0.3f, 0.3f, 0.3f});
  objShader.setVec3("directlight.specular", {0.2f, 0.2f, 0.2f});

  for(int i = 0; i < lampNum; ++i){
    objShader.setVec3("pointlights[" + std::to_string(i) + "].ambient",  {.1f, .1f, .1f});
    objShader.setVec3("pointlights[" + std::to_string(i) + "].diffuse",  {.5f, 0.5f, 0.5f});
    objShader.setVec3("pointlights[" + std::to_string(i) + "].specular", {1.f, 1.f, 1.f});
    objShader.setFloat("pointlights[" + std::to_string(i) + "].constant",  1.0f);
    objShader.setFloat("pointlights[" + std::to_string(i) + "].linear",    0.05f);
    objShader.setFloat("pointlights[" + std::to_string(i) + "].quadratic", 0.015f);
  }

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
  lampShader.setMatrix4fv("model", model);
  lampShader.setMatrix4fv("view", cam.GetViewMatrix());
  lampShader.setMatrix4fv("projection", app.getProjectionMatrix());

  app.generateNewVAOFromArray("objCube", dm.getCubeVerticesArray(), dm.getCubeVerticesArraySize(), {{0, 3}, {2, 2}, {3, 3}}, dm.getCubeIndicesArray(), dm.getCubeIndicesArraySize());
  app.generateNewVAOFromArray("lampSphere", dm.getSphereVerticesArray(), dm.getSphereVerticesArraySize(), {{0, 3}}, dm.getSphereIndicesArray(), dm.getSphereIndicesArraySize());

  app.run([=, &objShader, &lampShader, &cam, &app, &dm](float a_delta){
    cam.update(app.getWindow(), a_delta);

    lampShader.use();
    app.useVAO("lampSphere");
    std::vector<glm::vec3> rotateLampPosVec;
    for(int i = 0;i<lampNum;++i){
      glm::mat4 model = glm::mat4(1.0f);
      glm::vec3 rotateLampPos = glm::rotate(lampPosVec[i]-lampRotAxisVec[i]*lampPosVec[i], (float)glfwGetTime(), lampRotAxisVec[i]) + lampRotAxisVec[i]*lampPosVec[i];
      rotateLampPosVec.push_back(rotateLampPos);
      model = glm::translate(model, rotateLampPos);
      model = glm::scale(model, glm::vec3(0.2f));
      model = glm::rotate(model, (float)glfwGetTime(), glm::vec3(1, 1, 1));
      lampShader.setMatrix4fv("model", model);
      lampShader.setMatrix4fv("view", cam.GetViewMatrix());
      glDrawElements(GL_TRIANGLES, dm.getSphereVerticesCount(), GL_UNSIGNED_INT, 0);
    }

    objShader.use();
    app.useVAO("objCube");
    for(int i = 0; i< lampNum; ++i){
      objShader.setVec3("pointlights[" + std::to_string(i) + "].position", rotateLampPosVec[i]);
    }
    objShader.setVec3("cameraPos", cam.GetCameraPos());
    objShader.setVec3("spotlight.position", cam.GetCameraPos());
    objShader.setVec3("spotlight.direction", cam.GetCameraFront());

    for(int i = 0; i < cudeNum; i++) {
      glm::mat4 model = glm::mat4(1.0f);
      model = glm::translate(model, cubePositions[i]);
      model = glm::rotate(model, (float)glfwGetTime(), axis[i]);
      objShader.setMatrix4fv("model", model);
      objShader.setMatrix4fv("view", cam.GetViewMatrix());
      glDrawElements(GL_TRIANGLES, dm.getCubeVerticesCount(), GL_UNSIGNED_INT, 0);
    }
  });
}