#include "tinygl/glApp.h"
#include "tinygl/camera.h"
#include "tinygl/shader_s.h"
#include "tinygl/default_mesh.h"
#include "tinygl/SpriteRenderer.h"

#include "Logic.h"
#include "Input.h"

// new way
int main()
{
  GLApp app(800, 600);
  app.init();
  // with this config of the ortho, the origin is at the top-left
  glm::mat4 projection = glm::ortho(0.0f, (float)app.getScreenWidth(), (float)app.getScreenHeight(), 0.0f);
  app.setProjectionMatrix(projection);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  // init resource
  // VAO
  DefaultMesh dm;
  dm.initSpriteData(true); // flip here because auther seems not set stbi_set_flip_vertically_on_load(true);
  app.generateNewVAOFromArray("sprite", dm.getSpriteVerticesArray(), dm.getSpriteVerticesArraySize(), {{0, 4}}, dm.getSpriteIndicesArray(), dm.getSpriteIndicesArraySize());
  // texture
  app.loadTexture2D(ASSETS_2D_BASEPATH + "/awesomeface.png", 1);
  app.loadTexture2D(ASSETS_2D_BASEPATH + "/matrix.jpg", 2);
  app.loadTexture2D(ASSETS_2D_BASEPATH + "/bricks2.jpg", 3);
  app.loadTexture2D(ASSETS_2D_BASEPATH + "/brickwall.jpg", 4);
  app.loadTexture2D(ASSETS_2D_BASEPATH + "/paddle.png", 5);
  // shader
  Shader spriteShader(COMMON_SHADERS_BASEPATH + "/sprite.vs", COMMON_SHADERS_BASEPATH + "/sprite.fs");
  spriteShader.init("model");
  spriteShader.setMatrix4fv("projection", projection);

  // init core class
  std::shared_ptr<SpriteRenderer> spriteRenderer = std::make_shared<SpriteRenderer>(&app, &spriteShader, "sprite", dm.getSpriteVerticesCount());
  std::shared_ptr<GameLevel> level = std::make_shared<GameLevel>(spriteRenderer, 3, 4, glm::vec2(800, 300));
  std::shared_ptr<BouncingBall> ball = std::make_shared<BouncingBall>(spriteRenderer, 1, glm::vec2(400, 400), glm::vec2(30, 30), 300);
  std::shared_ptr<GameLogic> logic = std::make_shared<GameLogic>(level, ball, glm::vec2(800, 600));
  std::shared_ptr<GameInput> input = std::make_shared<GameInput>(spriteRenderer, 5, 500, 800, 600, 100, 10);

  app.run([=, &app](float a_delta){
    // background
    spriteRenderer->SetProperty({2, {1, 1, 1, 0.3}, {0, 0, -0.1}, {800, 600, 0}, 0, {1, 1, 1}});
    spriteRenderer->Draw();

    input->Update(app.getWindow(), a_delta);

    logic->Update(a_delta, input->GetPaddleAABB());
  });
}