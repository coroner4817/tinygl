#pragma once
#include "tinygl/glApp.h"
#include "tinygl/shader_s.h"
#include "tinygl/VAO_manager.h"

struct RenderPacket{
  GLuint texture;
  glm::vec4 color;
  glm::vec3 pos;
  glm::vec3 scl;
  float angle = 0;
  glm::vec3 axis = {1,1,1};
};

class SpriteRenderer : public VAOManager {
public:
  SpriteRenderer(GLApp* a, Shader *s, const std::string& vao, GLuint vertice_cnt)
    : app_(a), shader_(s), vao_name_(vao), vertice_cnt_(vertice_cnt) {}

  ~SpriteRenderer(){}

  void SetProperty(const RenderPacket& rp){
    texture_id_ = rp.texture;
    color_ = rp.color;
    pos_ = rp.pos;
    scl_ = rp.scl;
    angle_ = rp.angle;
    axis_ = rp.axis;
    is_init_ = true;
  }

  void Draw(){
    if(is_init_){
      shader_->use();{
        app_->useVAO(vao_name_);
        shader_->setInt("image", texture_id_); // uniform name is hardcoded right now
        shader_->setVec4("spriteColor", color_);
        shader_->setModelMatrix(pos_, scl_, angle_, axis_);
        DRAW(vertice_cnt_);
      }
    }
  }

private:
  GLApp *app_;
  Shader *shader_;

  std::string vao_name_;
  GLuint texture_id_;
  GLuint vertice_cnt_;

  bool is_init_ = false;

  glm::vec4 color_;
  glm::vec3 pos_;
  glm::vec3 scl_;
  float angle_;
  glm::vec3 axis_;
};