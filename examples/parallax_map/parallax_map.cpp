#include "tinygl/glApp.h"
#include "tinygl/camera.h"
#include "tinygl/shader_s.h"
#include "tinygl/default_mesh.h"

glm::vec3 lampPos = {0.5f, 1.0f, 0.3f};
void handcraftQuad(GLApp* app);

int main()
{
  GLApp app;
  app.init();
  app.setProjectionMatrix(45.f, 0.1f, 100.f);
  Camera cam;
  cam.addViewMatrixUpdateListener(&app);
  DefaultMesh dm(&app, &cam);
  dm.initSphere();

  app.loadTexture2D(ASSETS_2D_BASEPATH + "/wood.png", 1);
  app.loadTexture2D(ASSETS_2D_BASEPATH + "/toy_box_normal.png", 2);
  app.loadTexture2D(ASSETS_2D_BASEPATH + "/toy_box_disp.png", 3);

  handcraftQuad(&app);

  Shader tbnShader(COMMON_SHADERS_BASEPATH + "/tbn.vs", getExecutablePath() + "/parallex.fs");
  tbnShader.init("model", "view", &cam, "projection", app.getProjectionMatrix(), "viewPos");
  tbnShader.setVec3("lightPos", lampPos);
  tbnShader.setInt("diffuseMap", 1);
  tbnShader.setInt("normalMap", 2);
  tbnShader.setInt("dispMap", 3);
  tbnShader.setFloat("heightScale", 0.1);

  app.run([=, &tbnShader, &cam, &app, &dm](float a_delta){
    cam.update(app.getWindow(), a_delta);

    dm.drawSphere(glm::vec4(1, 1, 1, 0.8), lampPos, glm::vec3(0.1));

    tbnShader.use();
    app.useVAO("handcraftQuad");
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
  });
}

void handcraftQuad(GLApp* app)
{
  // positions
  glm::vec3 pos1(-1.0f,  1.0f, 0.0f);
  glm::vec3 pos2(-1.0f, -1.0f, 0.0f);
  glm::vec3 pos3( 1.0f, -1.0f, 0.0f);
  glm::vec3 pos4( 1.0f,  1.0f, 0.0f);
  // texture coordinates
  glm::vec2 uv1(0.0f, 1.0f);
  glm::vec2 uv2(0.0f, 0.0f);
  glm::vec2 uv3(1.0f, 0.0f);
  glm::vec2 uv4(1.0f, 1.0f);
  // normal vector
  glm::vec3 nm(0.0f, 0.0f, 1.0f);

  // tangent/bitangent vectors usually is not given in obj file. Need to be manully calc
  // calculate tangent/bitangent vectors of both triangles
  glm::vec3 tangent1, bitangent1;
  glm::vec3 tangent2, bitangent2;
  // triangle 1
  // ----------
  glm::vec3 edge1 = pos2 - pos1;
  glm::vec3 edge2 = pos3 - pos1;
  glm::vec2 deltaUV1 = uv2 - uv1;
  glm::vec2 deltaUV2 = uv3 - uv1;

  GLfloat f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

  tangent1.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
  tangent1.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
  tangent1.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);
  tangent1 = glm::normalize(tangent1);

  bitangent1.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
  bitangent1.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
  bitangent1.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);
  bitangent1 = glm::normalize(bitangent1);

  // triangle 2
  // ----------
  edge1 = pos3 - pos1;
  edge2 = pos4 - pos1;
  deltaUV1 = uv3 - uv1;
  deltaUV2 = uv4 - uv1;

  f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

  tangent2.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
  tangent2.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
  tangent2.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);
  tangent2 = glm::normalize(tangent2);


  bitangent2.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
  bitangent2.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
  bitangent2.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);
  bitangent2 = glm::normalize(bitangent2);

  unsigned int quadIndex[] = {
    0, 1, 2,
    3, 4, 5
  };

  float quadVertices[] = {
      // positions            // normal         // texcoords  // tangent                          // bitangent
      pos1.x, pos1.y, pos1.z, nm.x, nm.y, nm.z, uv1.x, uv1.y, tangent1.x, tangent1.y, tangent1.z, bitangent1.x, bitangent1.y, bitangent1.z,
      pos2.x, pos2.y, pos2.z, nm.x, nm.y, nm.z, uv2.x, uv2.y, tangent1.x, tangent1.y, tangent1.z, bitangent1.x, bitangent1.y, bitangent1.z,
      pos3.x, pos3.y, pos3.z, nm.x, nm.y, nm.z, uv3.x, uv3.y, tangent1.x, tangent1.y, tangent1.z, bitangent1.x, bitangent1.y, bitangent1.z,

      pos1.x, pos1.y, pos1.z, nm.x, nm.y, nm.z, uv1.x, uv1.y, tangent2.x, tangent2.y, tangent2.z, bitangent2.x, bitangent2.y, bitangent2.z,
      pos3.x, pos3.y, pos3.z, nm.x, nm.y, nm.z, uv3.x, uv3.y, tangent2.x, tangent2.y, tangent2.z, bitangent2.x, bitangent2.y, bitangent2.z,
      pos4.x, pos4.y, pos4.z, nm.x, nm.y, nm.z, uv4.x, uv4.y, tangent2.x, tangent2.y, tangent2.z, bitangent2.x, bitangent2.y, bitangent2.z
  };

  app->generateNewVAOFromArray("handcraftQuad", quadVertices, sizeof(quadVertices), {{0, 3}, {1, 3}, {2, 2}, {3, 3}, {4, 3}}, quadIndex, sizeof(quadIndex));
}