#pragma once

#include <vector>
#include "tinygl/SpriteRenderer.h"
#include <memory>


enum BrickType {
  EMPTY = 0,
  SOLID,
  BLOCK,
};

using LEVEL_DATA = std::vector<std::vector<int>>;

const LEVEL_DATA level1_data = {
  {1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2},
  {2, 1, 2, 1, 2, 1, 0, 0, 0, 0, 2, 1, 2, 1, 2, 1},
  {1, 2, 0, 0, 1, 2, 0, 0, 0, 0, 1, 2, 0, 0, 1, 2},
  {2, 1, 0, 0, 2, 1, 2, 1, 2, 1, 2, 1, 0, 0, 2, 1},
  {1, 2, 0, 0, 1, 2, 1, 2, 1, 2, 1, 2, 0, 0, 1, 2},
  {2, 1, 0, 0, 2, 1, 0, 0, 0, 0, 2, 1, 0, 0, 2, 1},
  {1, 2, 1, 2, 1, 2, 0, 0, 0, 0, 1, 2, 1, 2, 1, 2},
  {2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1},
};

class GameLevel {
public:
  GameLevel(std::shared_ptr<SpriteRenderer> renderer, GLuint block, GLuint solid, const glm::vec2& range){
    renderer_ = std::move(renderer);
    block_texture_ = block;
    solid_texture_ = solid;
    range_ = range;
  }

  void setLevel(int level){
    switch(level){
      case 1: setLevel(level1_data); break;
      default: setLevel(level1_data); break;
    }
  }

  void setLevel(const LEVEL_DATA& level){
    GLuint height = level.size();
    GLuint width = level[0].size();

    // assume the screen is 800 * 600
    GLuint unit_h = range_.y / height;
    GLuint unit_w = range_.x / width;

    for(int i = 0; i < level.size(); ++i){
      for(int j = 0; j < level[i].size(); ++j){
        BrickType type = static_cast<BrickType>(level[i][j]);

        RenderPacket rp;
        switch(type){
          case BrickType::EMPTY: continue;
          case BrickType::SOLID:{
            rp.texture = block_texture_;
          }break;
          case BrickType::BLOCK:{
            rp.texture = solid_texture_;
          }break;
          default: break;
        }

        rp.color = {1, 1, 1, 1};
        rp.pos = {j * unit_w, i * unit_h, 0};
        rp.scl = {unit_w, unit_h, 0};
        rp.angle = 0;
        rp.axis = {1, 1, 1};
        render_packet_.push_back(rp);
      }
    }
  }

  void Draw(){
    for(auto& packet : render_packet_){
      renderer_->SetProperty(packet);
      renderer_->Draw();
    }
  }

  // return reference so that we can modify the render packet
  std::vector<RenderPacket>* GetLevelInfo() {
    return &render_packet_;
  }

private:
  std::shared_ptr<SpriteRenderer> renderer_;
  GLuint block_texture_;
  GLuint solid_texture_;
  glm::vec2 range_;

  std::vector<RenderPacket> render_packet_;
};