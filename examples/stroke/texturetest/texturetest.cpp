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

  const std::string textureName = "Abstract_006";
  app.loadTexture2D(ASSETS_BASEPATH + "stroke/" + textureName + "/" + textureName + "_COLOR.jpg", 0);
  // app.loadTexture2D(ASSETS_BASEPATH + "stroke/Ring_001/Ring_001.png", 0);

  Shader objShader(getExecutablePath() + "/texturetest.vert", getExecutablePath() + "/texturetest.frag");
  objShader.use();
  objShader.setInt("ourTexture", 0);
  glm::mat4 model = glm::mat4(1.0f);
  objShader.setMatrix4fv("model", model);
  objShader.setMatrix4fv("view", cam.GetViewMatrix());
  objShader.setMatrix4fv("projection", app.getProjectionMatrix());

  // ----------------------------------------------------------
  float vertices[] = {
0.000000, 0.000000, -0.050000,
0.000000, 0.000000,

0.047553, 0.000000, -0.015451,
0.250000, 0.000000,

0.029389, 0.000000, 0.040451,
0.500000, 0.000000,

-0.029389, 0.000000, 0.040451,
0.750000, 0.000000,

-0.047553, 0.000000, -0.015451,
1.000000, 0.000000,

0.000000, 0.620711, -0.050000,
0.000000, 1.000000,

0.047553, 0.537302, -0.015451,
0.250000, 1.000000,

0.029389, 0.402343, 0.040451,
0.500000, 1.000000,

-0.029389, 0.402343, 0.040451,
0.750000, 1.000000,

-0.047553, 0.537302, -0.015451,
1.000000, 1.000000,

0.000000, 0.000000, 0.500000,
0.000000, 0.000000,

0.000000, 0.000000, 0.500000,
0.250000, 0.000000,

0.000000, 0.000000, 0.500000,
0.500000, 0.000000,

0.000000, 0.000000, 0.500000,
0.750000, 0.000000,

0.000000, 0.000000, 0.500000,
1.000000, 0.000000
  };
  unsigned int indices[60] = {0, 5, 1,
5, 6, 1,
1, 6, 2,
6, 7, 2,
2, 7, 3,
7, 8, 3,
3, 8, 4,
8, 9, 4,
4, 9, 0,
9, 5, 0,
5, 10, 6,
10, 11, 6,
6, 11, 7,
11, 12, 7,
7, 12, 8,
12, 13, 8,
8, 13, 9,
13, 14, 9,
9, 14, 5,
14, 10, 5};

  int vertexCnt = 60;

  glm::vec3 cubePositions[] = {
    glm::vec3( 2.0f,  -2.0f,  -5.0f),
    glm::vec3( -2.0f,  2.0f,  -5.0f),
    glm::vec3( -2.0f,  -2.0f,  -5.0f),
  };

  app.generateNewVAOFromArray("objCube", vertices, sizeof(vertices), {{0, 3}, {1, 2}}, indices, sizeof(indices));

  app.run([=, &objShader, &cam, &app](float a_delta){
    cam.update(app.getWindow(), a_delta);

    objShader.use();
    app.useVAO("objCube");
    objShader.setMatrix4fv("view", cam.GetViewMatrix());

    glDrawElements(GL_TRIANGLES, vertexCnt, GL_UNSIGNED_INT, 0);
  });
}