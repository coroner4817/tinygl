#pragma once

#include "tinygl/common.h"

struct AABB {
  glm::vec2 min;
  glm::vec2 max;
};

class BouncingBall {

public:
  BouncingBall(std::shared_ptr<SpriteRenderer> renderer, GLuint ball, const glm::vec2& pos, const glm::vec2& size, float speed){
    renderer_ = std::move(renderer);
    rp.texture = ball;
    rp.color = {1, 1, 1, 1};
    rp.pos = glm::vec3(pos, 0.1);
    rp.scl = glm::vec3(size, 0);

    size_ = rp.scl;
    speed_ = speed;
    cur_position_ = rp.pos - size_ / 2;

    cur_direction_ = {0, 1, 0};

    init_pos_ = cur_position_;
  }

  void Update(float a_delta){
    // update position
    cur_position_ += speed_ * a_delta * cur_direction_;

    // render
    rp.pos = cur_position_;
    renderer_->SetProperty(rp);
    renderer_->Draw();
    cur_aabb_ = {
      .min = glm::vec2(cur_position_.x, cur_position_.y),
      .max = glm::vec2(cur_position_.x, cur_position_.y) + glm::vec2(size_.x, size_.y),
    };
  }

  void OnCollide(bool isVertical, const glm::vec2& shift=glm::vec2()){
    srand(time(NULL));
    cur_position_ += glm::vec3(shift, 0);
    if(isVertical){
      cur_direction_.y = -cur_direction_.y;
    }else{
      cur_direction_.x = -cur_direction_.x;
    }
    cur_direction_ += glm::vec3((rand()%60 - 30)/100.f, (rand()%60 - 30)/100.f, 0);
    cur_direction_ = glm::normalize(cur_direction_);
  }

  AABB GetCurrentAABB() const {
    return cur_aabb_;
  }

  glm::vec2 GetSize() const {
    return {size_.x, size_.y};
  }

  glm::vec2 GetCenter() const {
    return (cur_aabb_.min + cur_aabb_.max) / 2.f;
  }

  float GetRadius() const {
    return size_.x / 2;
  }

  void Reset() {
    cur_position_ = init_pos_;
    cur_direction_ = {0, 1, 0};
  }

private:
  glm::vec3 cur_position_;
  glm::vec3 cur_direction_;
  glm::vec3 size_;
  float speed_;
  glm::vec3 init_pos_;

  std::shared_ptr<SpriteRenderer> renderer_;
  RenderPacket rp;
  AABB cur_aabb_;
};