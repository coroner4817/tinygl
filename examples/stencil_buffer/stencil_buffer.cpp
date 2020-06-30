#include "tinygl/glApp.h"
#include "tinygl/camera.h"
#include "tinygl/shader_s.h"
#include "tinygl/default_mesh.h"

// stencil is use for block and show fragment
int main()
{
  // ----------------------------------------------------------
  // user define data
  int cudeNum = 2;
  glm::vec3 cubePositions[] = {
    glm::vec3( -1.0f,  0.01f,  -1.0f),
    glm::vec3( 2.0f,  0.01f, 0.0f),
  };
  glm::vec3 spherePos = {-1, 0, 1};
  glm::vec3 portalPos = {1, 0, 2};
  // ----------------------------------------------------------

  GLApp app;
  app.init();
  app.setProjectionMatrix(45.f, 0.1f, 100.f);
  // skybox causing issues with stencil buffer
  // app.useSkyBox("sor_sea", 0);
  Camera cam;
  cam.addViewMatrixUpdateListener(&app);
  DefaultMesh dm;
  dm.initPlaneData();
  dm.initCubeData();
  dm.initSphereData(1, 15, 15);

  app.loadTexture2D(ASSETS_2D_BASEPATH + "/marble.jpg", 0);
  app.loadTexture2D(ASSETS_2D_BASEPATH + "/metal.png", 1);
  app.loadTexture2D(ASSETS_2D_BASEPATH + "/awesomeface.png", 2);

  Shader objShader(getExecutablePath() + "/objshader.vs", getExecutablePath() + "/objshader.fs");
  Shader stencilShader(getExecutablePath() + "/objshader.vs", getExecutablePath() + "/stencilshader.fs");
  Shader sphereShader(getExecutablePath() + "/sphere.vs", getExecutablePath() + "/sphere.fs");
  Shader portalShader(getExecutablePath() + "/portal.vs", getExecutablePath() + "/portal.fs");

  objShader.use();
  glm::mat4 model = glm::mat4(1.0f);
  objShader.setMatrix4fv("model", model);
  objShader.setMatrix4fv("view", cam.GetViewMatrix());
  objShader.setMatrix4fv("projection", app.getProjectionMatrix());

  stencilShader.use();
  stencilShader.setMatrix4fv("model", model);
  stencilShader.setMatrix4fv("view", cam.GetViewMatrix());
  stencilShader.setMatrix4fv("projection", app.getProjectionMatrix());

  sphereShader.use();
  sphereShader.setMatrix4fv("model", model);
  sphereShader.setMatrix4fv("view", cam.GetViewMatrix());
  sphereShader.setMatrix4fv("projection", app.getProjectionMatrix());

  portalShader.use();
  portalShader.setMatrix4fv("model", model);
  portalShader.setMatrix4fv("view", cam.GetViewMatrix());
  portalShader.setMatrix4fv("projection", app.getProjectionMatrix());
  portalShader.setFloat("radius", 0.3f);

  app.generateNewVAOFromArray("objCube", dm.getCubeVerticesArray(), dm.getCubeVerticesArraySize(), {{0, 3}, {1, 2}, {2, 3}}, dm.getCubeIndicesArray(), dm.getCubeIndicesArraySize());
  app.generateNewVAOFromArray("plane", dm.getPlaneVerticesArray(), dm.getPlaneVerticesArraySize(), {{0, 3}, {1, 2}, {2, 3}}, dm.getPlaneIndicesArray(), dm.getPlaneIndicesArraySize());
  app.generateNewVAOFromArray("sphere", dm.getSphereVerticesArray(), dm.getSphereVerticesArraySize(), {{0, 3}}, dm.getSphereIndicesArray(), dm.getSphereIndicesArraySize());

  // for transparency
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  app.run([=, &objShader, &stencilShader, &sphereShader, &portalShader, &cam, &app, &dm](float a_delta){
    cam.update(app.getWindow(), a_delta);

    objShader.use();
    // plane
    app.useVAO("plane");
    objShader.setInt("texture1", 1);
    objShader.setVec4("color", {1, 1, 1, 1});
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(0, -0.5, 0));
    objShader.setMatrix4fv("model", model);
    objShader.setMatrix4fv("view", cam.GetViewMatrix());
    glDrawElements(GL_TRIANGLES, dm.getPlaneVerticesCount(), GL_UNSIGNED_INT, 0);

    glEnable(GL_STENCIL_TEST);
    // if the stencil test pass, the replace the stencil buffer to reference value 1
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
    glClear(GL_STENCIL_BUFFER_BIT);

    // for the content
    app.useVAO("objCube");
    objShader.setInt("texture1", 0);
    // for each fragment drawing later, will always write 1 to the stencil buffer
    // this will get a mask of the current fragment on the NDC
    // 0xFF is just an operating and flag
    glStencilFunc(GL_ALWAYS, 1, 0xFF);
    // turn on stencil buffer writing
    glStencilMask(0xFF);
    // original cubes
    for(int i = 0; i < cudeNum; ++i){
      glm::mat4 model = glm::mat4(1.0f);
      model = glm::translate(model, cubePositions[i] + glm::vec3(0, 0.05, 0));
      objShader.setMatrix4fv("model", model);
      objShader.setMatrix4fv("view", cam.GetViewMatrix());
      glDrawElements(GL_TRIANGLES, dm.getCubeVerticesCount(), GL_UNSIGNED_INT, 0);
    }

    // for the contour effect
    stencilShader.use();
    // if stencil value of this fragment is not equal to 1 then stencil test pass, then we draw this slight larger cube
    glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
    // disable updating to the stencil buffer
    glStencilMask(0x00);
    // disable depth test for always have the border at the front
    // comment out because looks wrong from beaneth due to depth test off
    // glDisable(GL_DEPTH_TEST);
    // scaled up cubes
    for(int i = 0; i < cudeNum; ++i){
      glm::mat4 model = glm::mat4(1.0f);
      model = glm::translate(model, cubePositions[i] + glm::vec3(0, 0.05, 0));
      // slight bigger so that the bigger area will be the contour
      model = glm::scale(model, glm::vec3(1.1));
      stencilShader.setMatrix4fv("model", model);
      stencilShader.setMatrix4fv("view", cam.GetViewMatrix());
      glDrawElements(GL_TRIANGLES, dm.getCubeVerticesCount(), GL_UNSIGNED_INT, 0);
    }

    // enable write to stencil buffer
    glStencilMask(0xFF);
    // enable depth test
    // glEnable(GL_DEPTH_TEST);

    //stencil hide and show effect
    app.useVAO("plane");
    objShader.use();
    objShader.setInt("texture1", 1);
    objShader.setVec4("color", {0, 0, 0, 0});

    // draw the stencil face for 6 face of the cube
    glStencilFunc(GL_ALWAYS, 2, 0x02);
    model = glm::mat4(1.0f);
    model = glm::translate(model, spherePos + glm::vec3(0, 0.05, 0.5));
    model = glm::scale(model, glm::vec3(0.1));
    model = glm::rotate(model, (float)M_PI/2, glm::vec3(1, 0, 0));
    objShader.setMatrix4fv("model", model);
    glDrawElements(GL_TRIANGLES, dm.getPlaneVerticesCount(), GL_UNSIGNED_INT, 0);

    glStencilFunc(GL_ALWAYS, 4, 0x04);
    model = glm::mat4(1.0f);
    model = glm::translate(model, spherePos + glm::vec3(0.5, 0.05, 0));
    model = glm::scale(model, glm::vec3(0.1));
    model = glm::rotate(model, (float)M_PI/2, glm::vec3(0, 0, 1));
    objShader.setMatrix4fv("model", model);
    glDrawElements(GL_TRIANGLES, dm.getPlaneVerticesCount(), GL_UNSIGNED_INT, 0);

    glStencilFunc(GL_ALWAYS, 8, 0x08);
    model = glm::mat4(1.0f);
    model = glm::translate(model, spherePos + glm::vec3(-0.5, 0.05, 0));
    model = glm::scale(model, glm::vec3(0.1));
    model = glm::rotate(model, (float)M_PI/2, glm::vec3(0, 0, 1));
    objShader.setMatrix4fv("model", model);
    glDrawElements(GL_TRIANGLES, dm.getPlaneVerticesCount(), GL_UNSIGNED_INT, 0);

    glStencilFunc(GL_ALWAYS, 16, 0x10);
    model = glm::mat4(1.0f);
    model = glm::translate(model, spherePos + glm::vec3(0, 0.05, -0.5));
    model = glm::scale(model, glm::vec3(0.1));
    model = glm::rotate(model, (float)M_PI/2, glm::vec3(1, 0, 0));
    objShader.setMatrix4fv("model", model);
    glDrawElements(GL_TRIANGLES, dm.getPlaneVerticesCount(), GL_UNSIGNED_INT, 0);

    glStencilFunc(GL_ALWAYS, 32, 0x20);
    model = glm::mat4(1.0f);
    model = glm::translate(model, spherePos + glm::vec3(0, 0.55, 0));
    model = glm::scale(model, glm::vec3(0.1));
    objShader.setMatrix4fv("model", model);
    glDrawElements(GL_TRIANGLES, dm.getPlaneVerticesCount(), GL_UNSIGNED_INT, 0);

    glStencilFunc(GL_ALWAYS, 64, 0x40);
    model = glm::mat4(1.0f);
    model = glm::translate(model, spherePos + glm::vec3(0, -0.45, 0));
    model = glm::scale(model, glm::vec3(0.1));
    objShader.setMatrix4fv("model", model);
    glDrawElements(GL_TRIANGLES, dm.getPlaneVerticesCount(), GL_UNSIGNED_INT, 0);

    // disable updating to the stencil buffer
    // strange bug: need to comment out this. But we don't need to write the stencil anymore
    // glStencilMask(0x00);

    glDisable(GL_DEPTH_TEST);
    app.useVAO("sphere");
    sphereShader.use();
    model = glm::mat4(1.0f);
    model = glm::translate(model, spherePos + glm::vec3(0, 0.05, 0));
    model = glm::scale(model, glm::vec3(0.2f));
    sphereShader.setMatrix4fv("model", model);
    sphereShader.setMatrix4fv("view", cam.GetViewMatrix());

    glStencilFunc(GL_EQUAL, 2, 0x02);
    sphereShader.setVec4("color", {1, 0, 0, 1});
    glDrawElements(GL_TRIANGLES, dm.getSphereVerticesCount(), GL_UNSIGNED_INT, 0);

    glStencilFunc(GL_EQUAL, 4, 0x04);
    sphereShader.setVec4("color", {0, 1, 0, 1});
    glDrawElements(GL_TRIANGLES, dm.getSphereVerticesCount(), GL_UNSIGNED_INT, 0);

    glStencilFunc(GL_EQUAL, 8, 0x08);
    sphereShader.setVec4("color", {0, 0, 1, 1});
    glDrawElements(GL_TRIANGLES, dm.getSphereVerticesCount(), GL_UNSIGNED_INT, 0);

    glStencilFunc(GL_EQUAL, 16, 0x10);
    sphereShader.setVec4("color", {0, 1, 1, 1});
    glDrawElements(GL_TRIANGLES, dm.getSphereVerticesCount(), GL_UNSIGNED_INT, 0);

    glStencilFunc(GL_EQUAL, 32, 0x20);
    sphereShader.setVec4("color", {1, 0, 1, 1});
    glDrawElements(GL_TRIANGLES, dm.getSphereVerticesCount(), GL_UNSIGNED_INT, 0);

    glStencilFunc(GL_EQUAL, 64, 0x40);
    sphereShader.setVec4("color", {1, 1, 0, 1});
    glDrawElements(GL_TRIANGLES, dm.getSphereVerticesCount(), GL_UNSIGNED_INT, 0);

    // Portal effect
    app.useVAO("plane");
    portalShader.use();
    portalShader.setInt("texture1", 1);
    glStencilFunc(GL_ALWAYS, 3, 0xFF);
    model = glm::mat4(1.0f);
    float s = glm::sin((float)glfwGetTime())*0.5;
    model = glm::translate(model, portalPos + glm::vec3(0, 0.05, 0.75) + glm::vec3(s, 0, s*s));
    model = glm::scale(model, glm::vec3(0.05, 0.1, 0.1));
    model = glm::rotate(model, (float)M_PI/2, glm::vec3(1, 0, 0));
    portalShader.setMatrix4fv("model", model);
    portalShader.setMatrix4fv("view", cam.GetViewMatrix());
    glDrawElements(GL_TRIANGLES, dm.getPlaneVerticesCount(), GL_UNSIGNED_INT, 0);

    glEnable(GL_DEPTH_TEST);

    glStencilFunc(GL_EQUAL, 3, 0xFF);
    app.useVAO("objCube");
    objShader.use();
    objShader.setInt("texture1", 2);
    objShader.setVec4("color", {1, 1, 1, 1});
    model = glm::mat4(1.0f);
    model = glm::translate(model, portalPos + glm::vec3(0, 0.05, 0));
    objShader.setMatrix4fv("model", model);
    objShader.setMatrix4fv("view", cam.GetViewMatrix());
    glDrawElements(GL_TRIANGLES, dm.getCubeVerticesCount(), GL_UNSIGNED_INT, 0);

    glDisable(GL_STENCIL_TEST);
  });
}
