#include "tinygl/glApp.h"
#include "tinygl/camera.h"
#include "tinygl/shader_s.h"

int main()
{
  // ----------------------------------------------------------
  // user define data
  float quadVertices[] = {
      // positions     // colors
      -0.05f,  0.05f,  1.0f, 0.0f, 0.0f,
      0.05f, -0.05f,  0.0f, 1.0f, 0.0f,
      -0.05f, -0.05f,  0.0f, 0.0f, 1.0f,

      -0.05f,  0.05f,  1.0f, 0.0f, 0.0f,
      0.05f, -0.05f,  0.0f, 1.0f, 0.0f,
      0.05f,  0.05f,  0.0f, 1.0f, 1.0f
  };

  glm::vec2 translations[100];
  int index = 0;
  float offset = 0.1f;
  for (int y = -10; y < 10; y += 2)
  {
    for (int x = -10; x < 10; x += 2)
    {
      glm::vec2 translation;
      translation.x = (float)x / 10.0f + offset;
      translation.y = (float)y / 10.0f + offset;
      translations[index++] = translation;
    }
  }
  // ----------------------------------------------------------

  GLApp app;
  app.init();

  Shader shader(getExecutablePath() + "/instanceShader.vs", getExecutablePath() + "/instanceShader.fs");

  app.generateNewVAOFromArray("quad", quadVertices, sizeof(quadVertices), {{0, 2}, {1, 3}});
  app.useVAO("quad");
  // pass the offset to shader through the buffer
  unsigned int instanceVBO;
  glGenBuffers(1, &instanceVBO);
  glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2) * 100, &translations[0], GL_STATIC_DRAW);
  glEnableVertexAttribArray(2);
  glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  // tell OpenGL this is an instanced vertex attribute !!!!
  glVertexAttribDivisor(2, 1);
  glBindVertexArray(0);

  app.run([=, &shader, &app](float a_delta){
    shader.use();
    app.useVAO("quad");
    glDrawArraysInstanced(GL_TRIANGLES, 0, 6, 100);
  });
}