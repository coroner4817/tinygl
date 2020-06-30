#pragma once

#include "tinygl/common.h"
#include "Level.h"
#include "Ball.h"

enum GameState {
  IDLE = 0,
  ACTIVE,
  END,
};

class GameLogic {
public:
  GameLogic(std::shared_ptr<GameLevel> level, std::shared_ptr<BouncingBall> ball, const glm::vec2& scene_size){
    level_ = std::move(level);
    level_->setLevel(1);
    ball_ = std::move(ball);
    state_ = GameState::ACTIVE;
    scene_size_ = scene_size;
  };
  ~GameLogic(){};

  void Update(float a_delta, const AABB& paddleAABB){
    if(state_ == GameState::ACTIVE){
      level_->Draw();
      ball_->Update(a_delta);

      ball_aabb = ball_->GetCurrentAABB();
      ball_center = ball_->GetCenter();
      ball_radius = ball_->GetRadius();

      if(checkLevelCollision()) return;
      if(checkPaddleCollision(paddleAABB)) return;
      if(checkHittingWall()) return;
    }
  };

  glm::vec2 VectorDirection(const glm::vec2& direction)
  {
    static const glm::vec2 compass[] = {
      glm::vec2(0.0f, 1.0f),	// up
      glm::vec2(1.0f, 0.0f),	// right
      glm::vec2(0.0f, -1.0f),	// down
      glm::vec2(-1.0f, 0.0f)	// left
    };
    float max = 0.0f;
    int best_match = -1;
    for(int i = 0; i < 4; i++){
      float dot_product = glm::dot(glm::normalize(direction), compass[i]);
      if(dot_product > max){
        max = dot_product;
        best_match = i;
      }
    }
    return compass[best_match];
  }

  bool checkBallBrickCollision(const glm::vec2& ball_center, const float ball_radius, const AABB& brick_aabb, bool& isVertical, glm::vec2& shift)
  {
    glm::vec2 brick_center = (brick_aabb.min + brick_aabb.max) / 2.f;
    glm::vec2 brick_size = (brick_aabb.max - brick_aabb.min);
    glm::vec2 diff = ball_center - brick_center;
    glm::vec2 projection = glm::clamp(diff, -brick_size/2, brick_size/2) + brick_center;
    float dist = ball_radius - glm::length(projection - ball_center);
    bool ret = (dist >= 0);
    shift = VectorDirection(diff) * dist;
    if(ret){
      auto dist_min = glm::length(projection - brick_aabb.min);
      auto dist_max = glm::length(projection - brick_aabb.max);

      std::function<bool(const glm::vec2&, const glm::vec2&)> checkIsVertical = [=](const glm::vec2& pos, const glm::vec2& ref){
        float dis_x = abs(pos.x - ref.x);
        float dis_y = abs(pos.y - ref.y);
        return dis_y <= dis_x;
      };

      if(dist_max > dist_min){
        isVertical = checkIsVertical(projection, brick_aabb.min);
      }else{
        isVertical = checkIsVertical(projection, brick_aabb.max);
      }
    }
    return ret;
  }

  // TODO: this has funny result
  bool checkPaddleCollision(const AABB& paddle){
    bool is_vertical; glm::vec2 shift;
    if(checkBallBrickCollision(ball_center, ball_radius, paddle, is_vertical, shift)){
      ball_->OnCollide(true, shift);
      return true;
    }
    return false;
  }

  bool checkLevelCollision(){
    auto bricks = level_->GetLevelInfo();
    size_t size_before = bricks->size();
    bricks->erase(
      std::remove_if(
        bricks->begin(), bricks->end(),
        [=](const RenderPacket& rp){
          auto bmax = (rp.pos + rp.scl);
          AABB brick_aabb = {
            .min = glm::vec2(rp.pos.x, rp.pos.y),
            .max = glm::vec2(bmax.x, bmax.y)
          };
          bool isVertical; glm::vec2 shift;
          if(checkBallBrickCollision(ball_center, ball_radius, brick_aabb, isVertical, shift)){
            ball_->OnCollide(isVertical, shift);
            return true;
          }
          return false;
        }
      ),
      bricks->end()
    );

    return size_before != bricks->size();
  }

  bool checkHittingWall(){
    if(ball_center.y - ball_radius < 0){
      ball_->OnCollide(true);
      return true;
    }else if(ball_center.x + ball_radius > scene_size_.x){
      ball_->OnCollide(false);
      return true;
    }else if(ball_center.x - ball_radius < 0){
      ball_->OnCollide(false);
      return true;
    }else if(ball_center.y > scene_size_.y){
      // dead
      level_->setLevel(1);
      ball_->Reset();
      return true;
    }
    return false;
  }

private:
  GameState state_;
  std::shared_ptr<GameLevel> level_;
  std::shared_ptr<BouncingBall> ball_;
  glm::vec2 scene_size_;

  AABB ball_aabb;
  glm::vec2 ball_center;
  float ball_radius;
};