#include "tinygl/glApp.h"
#include "tinygl/camera.h"
#include "tinygl/shader_s.h"
#include "tinygl/model_loader.h"

// user define vertex structure, because only user know the input model vertex data format
struct Vertex
{
  glm::vec3 Position;
  glm::vec3 Normal;
  glm::vec2 TexCoords;
  glm::vec3 Tangent;
  glm::vec3 Bitangent;
};

// user defined load texture type
std::vector<aiTextureType> asteroidTextureTypes = {aiTextureType_DIFFUSE};

int main()
{
  GLApp app;
  app.init();
  app.setProjectionMatrix(45.f, 0.1f, 100.f);
  Camera cam;
  cam.addViewMatrixUpdateListener(&app);

  auto modelLoader = ModelLoader<Vertex>(5);
  modelLoader.setProcessMeshHandler(STANDARD_OBJ_LOADER(Vertex, asteroidTextureTypes));

  modelLoader.load(ASSETS_3D_BASEPATH + "asteroid/rock/rock.obj", "rock");
  modelLoader.load(ASSETS_3D_BASEPATH + "asteroid/planet/planet.obj", "planet");

  Shader modelShader(getExecutablePath() + "/asteroid.vs", getExecutablePath() + "/asteroid.fs");
  modelShader.init("model", "view", &cam, "projection", app.getProjectionMatrix());
  modelShader.setTextureBaseName(aiTextureType_DIFFUSE, "texture_diffuse");

  // generate a large list of semi-random model transformation matrices
  // ------------------------------------------------------------------
  unsigned int amount = 1000;
  glm::mat4* modelMatrices;
  modelMatrices = new glm::mat4[amount];
  srand(glfwGetTime()); // initialize random seed
  float radius = 50.0;
  float offset = 2.5f;
  for (unsigned int i = 0; i < amount; i++)
  {
    glm::mat4 model = glm::mat4(1.0f);
    // 1. translation: displace along circle with 'radius' in range [-offset, offset]
    float angle = (float)i / (float)amount * 360.0f;
    float displacement = (rand() % (int)(2 * offset * 100)) / 100.0f - offset;
    float x = sin(angle) * radius + displacement;
    displacement = (rand() % (int)(2 * offset * 100)) / 100.0f - offset;
    float y = displacement * 0.4f; // keep height of asteroid field smaller compared to width of x and z
    displacement = (rand() % (int)(2 * offset * 100)) / 100.0f - offset;
    float z = cos(angle) * radius + displacement;
    model = glm::translate(model, glm::vec3(x, y, z));
    // 2. scale: Scale between 0.05 and 0.25f
    float scale = (rand() % 20) / 100.0f + 0.05;
    model = glm::scale(model, glm::vec3(scale));
    // 3. rotation: add random rotation around a (semi)randomly picked rotation axis vector
    float rotAngle = (rand() % 360);
    model = glm::rotate(model, rotAngle, glm::vec3(0.4f, 0.6f, 0.8f));
    // 4. now add to list of matrices
    modelMatrices[i] = model;
  }

  // bind to the VAO
  unsigned int buffer;
  glGenBuffers(1, &buffer);
  glBindBuffer(GL_ARRAY_BUFFER, buffer);
  glBufferData(GL_ARRAY_BUFFER, amount * sizeof(glm::mat4), &modelMatrices[0], GL_STATIC_DRAW);

  auto it = modelLoader.GetVAOIteratorBegin("rock");
  auto end = modelLoader.GetVAOIteratorEnd("rock");
  for (; it != end; it++)
  {
    glBindVertexArray(*it);
    // vertex Attributes
    GLsizei vec4Size = sizeof(glm::vec4);
    // in the vertex shader the mat4 begin at 5
    glEnableVertexAttribArray(5);
    // mat4 use 4 data layout
    glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)0);
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)(vec4Size));
    glEnableVertexAttribArray(5);
    glVertexAttribPointer(7, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)(2 * vec4Size));
    glEnableVertexAttribArray(6);
    glVertexAttribPointer(8, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)(3 * vec4Size));

    glVertexAttribDivisor(5, 1);
    glVertexAttribDivisor(6, 1);
    glVertexAttribDivisor(7, 1);
    glVertexAttribDivisor(8, 1);

    glBindVertexArray(0);
  }

  app.run([=, &modelShader, &modelLoader, &cam, &app](float a_delta){
    cam.update(app.getWindow(), a_delta);

    modelShader.use();
    modelShader.setModelMatrix(glm::vec3(0.0f, -3.0f, 0.0f), glm::vec3(2.f, 2.f, 2.f));
    modelLoader.Draw({"planet"}, modelShader);

    modelShader.use();
    auto it = modelLoader.GetVAOIteratorBegin("rock");
    auto end = modelLoader.GetVAOIteratorEnd("rock");
    for (; it != end; it++){
      glBindVertexArray(*it);
      modelShader.setMatrix4fv("model", glm::mat4(1));
      // TODO: not finished, headache
      // glDrawElementsInstanced(GL_TRIANGLES, (*it).indices.size(), GL_UNSIGNED_INT, 0, amount);
      // modelLoader.Draw({"rock"}, modelShader);
    }
  });
}