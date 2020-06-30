#include "tinygl/glApp.h"
#include "tinygl/camera.h"
#include "tinygl/shader_s.h"
#include "tinygl/default_mesh.h"
#include "tinygl/SpriteRenderer.h"

// new way
int main()
{
  GLApp app(800, 600);
  app.init();
  // with this config of the ortho, the origin is at the top-left
  glm::mat4 projection = glm::ortho(0.0f, (float)app.getScreenWidth(), (float)app.getScreenHeight(), 0.0f);
  app.setProjectionMatrix(projection);
  // VAO
  DefaultMesh dm;
  dm.initSpriteData(true); // flip here because auther seems not set stbi_set_flip_vertically_on_load(true);
  app.generateNewVAOFromArray("sprite", dm.getSpriteVerticesArray(), dm.getSpriteVerticesArraySize(), {{0, 4}}, dm.getSpriteIndicesArray(), dm.getSpriteIndicesArraySize());
  // texture
  app.loadTexture2D(ASSETS_2D_BASEPATH + "/awesomeface.png", 1);
  // shader
  Shader spriteShader(COMMON_SHADERS_BASEPATH + "/sprite.vs", COMMON_SHADERS_BASEPATH + "/sprite.fs");
  spriteShader.init("model");
  spriteShader.setMatrix4fv("projection", projection);

  SpriteRenderer face_renderer(
    &app, &spriteShader, "sprite", dm.getSpriteVerticesCount());
  face_renderer.SetProperty({1, {0, 1, 0, 1}, {200, 200, 0}, {300, 400, 0}, 45, {0, 0, 1}});

  app.run([=, &face_renderer](float a_delta){
    face_renderer.Draw();
  });
}