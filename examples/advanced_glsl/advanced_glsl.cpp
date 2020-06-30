#include "tinygl/glApp.h"
#include "tinygl/camera.h"
#include "tinygl/shader_s.h"
#include "tinygl/default_mesh.h"

int main()
{
  // ----------------------------------------------------------
  // user define data
  glm::vec3 cubePositions[] = {
    glm::vec3( -0.75f,  0.75f,  0.0f),
    glm::vec3( 0.75f,  0.75f, 0.0f),
    glm::vec3(-0.75f, -0.75f, 0.0f),
    glm::vec3(0.75f, -0.75f, 0.0f),
  };
  // ----------------------------------------------------------

  GLApp app;
  app.init();
  app.setProjectionMatrix(45.f, 0.1f, 100.f);
  Camera cam;
  cam.addViewMatrixUpdateListener(&app);
  DefaultMesh dm;
  dm.initPlaneData();
  dm.initCubeData();

  Shader shaderRed(getExecutablePath() + "/uniformbuffer.vs", getExecutablePath() + "/red.fs");
  Shader shaderGreen(getExecutablePath() + "/uniformbuffer.vs", getExecutablePath() + "/green.fs");
  Shader shaderBlue(getExecutablePath() + "/uniformbuffer.vs", getExecutablePath() + "/blue.fs");
  Shader shaderYellow(getExecutablePath() + "/uniformbuffer.vs", getExecutablePath() + "/yellow.fs");

  app.generateNewVAOFromArray("objCube", dm.getCubeVerticesArray(), dm.getCubeVerticesArraySize(), {{0, 3}, {1, 2}, {2, 3}}, dm.getCubeIndicesArray(), dm.getCubeIndicesArraySize());
  app.generateNewVAOFromArray("plane", dm.getPlaneVerticesArray(), dm.getPlaneVerticesArraySize(), {{0, 3}, {1, 2}, {2, 3}}, dm.getPlaneIndicesArray(), dm.getPlaneIndicesArraySize());

  // uniform buffer object
  // benefit using shared memory save memory allocation
  // ---------------------------------
  shaderRed.bindUniformBlock("Matrices", 0);
  shaderGreen.bindUniformBlock("Matrices", 0);
  shaderBlue.bindUniformBlock("Matrices", 0);
  shaderYellow.bindUniformBlock("Matrices", 0);

  app.generateUBO("viewproj", 0, 2 * sizeof(glm::mat4));
  app.setUBO("viewproj", 0, sizeof(glm::mat4), glm::value_ptr(app.getProjectionMatrix()));

  app.run([=, &shaderRed, &shaderGreen, &shaderBlue, &shaderYellow, &cam, &app, &dm](float a_delta){
    cam.update(app.getWindow(), a_delta);

    // set the view and projection matrix in the uniform block - we only have to do this once per loop iteration.
    app.setUBO("viewproj", sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(cam.GetViewMatrix()));

    // cubes
    app.useVAO("objCube");
    shaderRed.use();
    shaderRed.setMatrix4fv("model", GetModelMat(cubePositions[0], glm::vec3(1), 0, glm::vec3()));
    glDrawElements(GL_TRIANGLES, dm.getCubeVerticesCount(), GL_UNSIGNED_INT, 0);

    shaderGreen.use();
    shaderGreen.setMatrix4fv("model", GetModelMat(cubePositions[1], glm::vec3(1), 0, glm::vec3()));
    glDrawElements(GL_TRIANGLES, dm.getCubeVerticesCount(), GL_UNSIGNED_INT, 0);

    shaderBlue.use();
    shaderBlue.setMatrix4fv("model", GetModelMat(cubePositions[2], glm::vec3(1), 0, glm::vec3()));
    glDrawElements(GL_TRIANGLES, dm.getCubeVerticesCount(), GL_UNSIGNED_INT, 0);

    shaderYellow.use();
    shaderYellow.setMatrix4fv("model", GetModelMat(cubePositions[3], glm::vec3(1), 0, glm::vec3()));
    glDrawElements(GL_TRIANGLES, dm.getCubeVerticesCount(), GL_UNSIGNED_INT, 0);
  });
}