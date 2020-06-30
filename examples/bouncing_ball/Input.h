#pragma once

#include "tinygl/common.h"

class GameInput {

public:
  GameInput(std::shared_ptr<SpriteRenderer> renderer, GLuint paddle, float speed, float window_w, float window_h, float paddle_w, float paddle_h){
    renderer_ = std::move(renderer);
    speed_ = speed;
    paddle_pos_ = {(window_w - paddle_w) / 2, window_h - paddle_h, 0};
    window_width_ = window_w;
    paddle_width_ = paddle_w;
    paddle_height_ = paddle_h;

    rp.texture = paddle;
    rp.color = {1, 1, 1, 1};
    rp.pos = paddle_pos_;
    rp.scl = {paddle_w, paddle_h, 0};
  }

  void Update(GLFWwindow *window, float a_delta){
    // handle the input and update the paddle position
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS){
      glfwSetWindowShouldClose(window, true);
    }
    if(glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS){
      paddle_pos_ += speed_ * a_delta * glm::vec3(-1, 0, 0);
    }
    if(glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS){
      paddle_pos_ += speed_ * a_delta * glm::vec3(1, 0, 0);
    }

    if(paddle_pos_.x <= 0){
      paddle_pos_.x = 0;
    }else if(paddle_pos_.x + paddle_width_ >= window_width_){
      paddle_pos_.x = window_width_ - paddle_width_;
    }

    rp.pos = paddle_pos_;
    renderer_->SetProperty(rp);
    renderer_->Draw();
  }

  AABB GetPaddleAABB() const {
    return {
      .min = glm::vec2(paddle_pos_.x, paddle_pos_.y),
      .max = glm::vec2(paddle_pos_.x, paddle_pos_.y) + glm::vec2(paddle_width_, paddle_height_),
    };
  }

private:
  float speed_;
  std::shared_ptr<SpriteRenderer> renderer_;

  glm::vec3 paddle_pos_;
  float window_width_;
  float paddle_width_;
  float paddle_height_;
  RenderPacket rp;
};