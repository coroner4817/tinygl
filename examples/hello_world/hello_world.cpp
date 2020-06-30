#include "tinygl/glApp.h"
#include "tinygl/camera.h"
#include "tinygl/shader_s.h"
#include "tinygl/default_mesh.h"

// ----------------------------------------------------------
// user define data
int cudeNum = 2;
glm::vec3 cubePositions[] = {
  glm::vec3( -1.0f,  0.0f,  -1.0f),
  glm::vec3( 2.0f,  0.0f, 0.0f),
};
// ----------------------------------------------------------
// new way
int main()
{
  GLApp app;
  app.init();
  app.setProjectionMatrix(45.f, 0.1f, 100.f);
  Camera cam;
  cam.addViewMatrixUpdateListener(&app);
  DefaultMesh dm(&app, &cam);
  dm.initPlane();
  dm.initCube();

  app.loadTexture2D(ASSETS_2D_BASEPATH + "/container.jpg", 1);
  app.loadTexture2D(ASSETS_2D_BASEPATH + "/metal.png", 2);

  app.run([=, &cam, &app, &dm](float a_delta){
    cam.update(app.getWindow(), a_delta);

    // plane
    dm.drawPlane(2, glm::vec3(0, -0.51, 0));

    // cubes
    for(int i = 0; i < cudeNum; ++i){
      dm.drawCube(1, cubePositions[i], glm::vec3(1));
    }
  });
}


// // old way
// int main()
// {
//   GLApp app;
//   app.init();
//   app.setProjectionMatrix(45.f, 0.1f, 100.f);
//   Camera cam;
//   cam.addViewMatrixUpdateListener(&app);
//   DefaultMesh dm;
//   dm.initPlaneData();
//   dm.initCubeData();

//   app.loadTexture2D(ASSETS_2D_BASEPATH + "/container.jpg", 1);
//   app.loadTexture2D(ASSETS_2D_BASEPATH + "/metal.png", 2);

//   Shader stdShader(COMMON_SHADERS_BASEPATH + "/stdShader.vs", COMMON_SHADERS_BASEPATH + "/stdShader.fs");
//   stdShader.init("model", "view", &cam, "projection", app.getProjectionMatrix());
//   stdShader->setInt("isColor", false);

//   app.generateNewVAOFromArray("objCube", dm.getCubeVerticesArray(), dm.getCubeVerticesArraySize(), {{0, 3}, {1, 2}, {2, 3}}, dm.getCubeIndicesArray(), dm.getCubeIndicesArraySize());
//   app.generateNewVAOFromArray("plane", dm.getPlaneVerticesArray(), dm.getPlaneVerticesArraySize(), {{0, 3}, {1, 2}, {2, 3}}, dm.getPlaneIndicesArray(), dm.getPlaneIndicesArraySize());

//   app.run([=, &stdShader, &cam, &app, &dm](float a_delta){
//     cam.update(app.getWindow(), a_delta);

//     // plane
//     stdShader.use();
//     app.useVAO("plane");
//     stdShader.setInt("texture1", 2);
//     stdShader.setModelMatrix(glm::vec3(0, -0.51, 0), glm::vec3(1), 0, glm::vec3());
//     glDrawElements(GL_TRIANGLES, dm.getPlaneVerticesCount(), GL_UNSIGNED_INT, 0);

//     // cubes
//     stdShader.use();
//     app.useVAO("objCube");
//     stdShader.setInt("texture1", 1);
//     for(int i = 0; i < cudeNum; ++i){
//       stdShader.setModelMatrix(cubePositions[i], glm::vec3(1), 0, glm::vec3());
//       glDrawElements(GL_TRIANGLES, dm.getCubeVerticesCount(), GL_UNSIGNED_INT, 0);
//     }
//   });
// }