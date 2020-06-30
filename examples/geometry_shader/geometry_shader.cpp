#include "tinygl/glApp.h"
#include "tinygl/camera.h"
#include "tinygl/shader_s.h"

int main()
{
  // ----------------------------------------------------------
  // user define data
  float points[] = {
    // NDC        // color
    -0.5f,  0.5f, 1.0f, 0.0f, 0.0f, // top-left
     0.5f,  0.5f, 0.0f, 1.0f, 0.0f, // top-right
     0.5f, -0.5f, 0.0f, 0.0f, 1.0f, // bottom-right
    -0.5f, -0.5f, 1.0f, 1.0f, 0.0f  // bottom-left
  };
  // ----------------------------------------------------------

  GLApp app;
  app.init();
  app.setProjectionMatrix(45.f, 0.1f, 100.f);
  Camera cam;
  cam.addViewMatrixUpdateListener(&app);

  Shader geoShader(getExecutablePath() + "/geometry.vs", getExecutablePath() + "/geometry.fs", getExecutablePath() + "/geometry.gs");

  app.generateNewVAOFromArray("points", points, sizeof(points), {{0, 2}, {1, 3}});

  app.run([=, &geoShader, &cam, &app](float a_delta){
    cam.update(app.getWindow(), a_delta);

    // points
    geoShader.use();
    app.useVAO("points");
    glDrawArrays(GL_POINTS, 0, 4);
  });
}