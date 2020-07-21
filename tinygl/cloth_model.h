#pragma once

#include "tinygl/common.h"

// base on https://threejs.org/examples/#webgl_animation_cloth

struct Particle {
  int u;
  int v;

  glm::vec3 orig_pos;
  glm::vec3 curr_pos;
  glm::vec3 prev_pos;
  glm::vec3 accelerate;

  glm::vec2 tex_coor;
  glm::vec3 normal;
  float mass;
  float inv_mass;
  int shared_count;
  glm::vec3 gravity;

  Particle(int a_u, int a_v, glm::vec3 a_orig_pos, glm::vec2 a_tex_coor, float a_mass)
  : u(a_u), v(a_v)
  , orig_pos(a_orig_pos)
  , curr_pos(a_orig_pos)
  , prev_pos(a_orig_pos)
  , accelerate(glm::vec3(0))
  , tex_coor(a_tex_coor)
  , normal(glm::vec3(0))
  , mass(a_mass)
  , inv_mass(1/mass) {
    gravity = glm::vec3(0, -9.81, 0) * mass;
  };
  void addForce(glm::vec3 force){
    // a = f/m
    accelerate += force * inv_mass;
  };
  void integrate(float a_delta){
    a_delta = 0.18f;
    // first part is inertia effect plus energy conservation
    // second part is the extra force's accelerate effect on this frame
    glm::vec3 new_pos = (curr_pos - prev_pos) * 0.97f + curr_pos
                        + accelerate * a_delta * a_delta;
    prev_pos = curr_pos;
    curr_pos = new_pos;
    accelerate = glm::vec3(0);
  };
};

class Cloth {
public:
  // construct a cloth with stacks*slices, each square size is step
  // notice that the cloth actually has (stacks+1)*(slices+1) particles
  // the center of the cloth is at origin
  void init(int slices, int stacks, float step, float particle_mass) {
    slices_ = slices;
    stacks_ = stacks;
    width_ = slices * step;
    height_ = stacks * step;
    step_ = step;

    // build the cloth row-wise
    for(int j = 0; j < stacks + 1; ++j) {
      for(int i = 0; i < slices + 1; ++i) {
        glm::vec3 pos = {(i/(float)slices - 0.5f) * width_, (0.5f - j/(float)stacks) * height_, 0};
        particles_.push_back(
          // assume the pins of cloth is the top row
          // mass will times a factor so that more far from pins, harder to change the state
          Particle(i, j, pos, {i/(float)slices_, 1.0f - j/(float)stacks_}, particle_mass * log2f(3 * j + 2))
        );
      }
    }

    // pre-allocate
    vbo_.resize((stacks_ + 1 ) * (slices_ + 1) * 8);

    // generate ebo
    for(int j = 0; j < stacks_; ++j) {
      for(int i = 0; i < slices_; ++i) {
        // 2 CCW triangle per square
        // vbo is in row-wise
        int p = j * (slices_ + 1) + i;
        ebo_.push_back(p);
        ebo_.push_back(p + slices_ + 1);
        ebo_.push_back(p + 1);

        ebo_.push_back(p + slices_ + 1);
        ebo_.push_back(p + slices_ + 2);
        ebo_.push_back(p + 1);
      }
    }

    std::function<int(int, int)> index = [=](int u, int v) {
      return u + v * (slices_ + 1);
    };
    // set up constraint
    for(int j = 0; j < stacks_; ++j) {
      for(int i = 0; i < slices_; ++i) {
        constraint_pairs_.push_back(
          std::make_pair(index(i, j), index(i, j + 1))
        );
        constraint_pairs_.push_back(
          std::make_pair(index(i, j), index(i + 1, j))
        );
      }
    }
    for(int j = 0; j < stacks_; ++j) {
      constraint_pairs_.push_back(
        std::make_pair(index(slices_, j), index(slices_, j + 1))
      );
    }
    for(int i = 0; i < slices_; ++i) {
      constraint_pairs_.push_back(
        std::make_pair(index(i, stacks_), index(i + 1, stacks_))
      );
    }

    getUpdatedVBO();
  };

  bool update(float a_delta) {
    now_ += a_delta;
    // aero force
    {
      float wind_strength = std::cos(now_ / 7) * 2 + 10;
      glm::vec3 wind_force = {std::sin(now_/2), std::cos(now_/3), std::sin(now_/1)};
      wind_force = glm::normalize(wind_force) * wind_strength;
      // add force per index
      for(auto& i : ebo_) {
        particles_[i].addForce(
          particles_[i].normal * glm::dot(particles_[i].normal, wind_force)
        );
      }
    }
    // gravity
    {
      for(int i = 0; i < particles_.size(); ++i) {
        particles_[i].addForce(particles_[i].gravity);
        particles_[i].integrate(a_delta);
      }
    }
    // cloth constraint
    for(auto& i : constraint_pairs_) {
      glm::vec3 diff = particles_[i.second].curr_pos - particles_[i.first].curr_pos;
      float curr_distance = glm::length(diff);
      if(glm::epsilonEqual(curr_distance, 0.f, 0.001f)){
        continue;
      }
      glm::vec3 correction = diff * (1 - step_ / curr_distance) * 0.5f;
      particles_[i.first].curr_pos += correction;
      particles_[i.second].curr_pos -= correction;
    }
    // pin constrain
    {
      // pins are the first row
      for(int i = 0; i < slices_ + 1; ++i) {
        particles_[i].curr_pos = particles_[i].orig_pos;
        particles_[i].prev_pos = particles_[i].orig_pos;
      }
    }

    is_dirty_ = true;
    return is_dirty_;
  };
  // build kTriangles layout data, pos/uv/normal
  std::vector<float> getUpdatedVBO() {
    if(!is_dirty_) return vbo_;

    // 1, calculate the current normal
    int vbo_idx = 0;
    for(int j = 0; j < stacks_ + 1; ++j) {
      for(int i = 0; i < slices_ + 1; ++i) {
        Particle& p = particles_[j * (slices_ + 1) + i];

        // normal is calculated base on particles around
        glm::vec3 normal_sum(0);
        // CCW from the top particles, CCW because right hand system
        std::vector<std::pair<int, glm::vec3>> particles_around;
        if(j - 1 >= 0) {
          particles_around.push_back(
            std::make_pair(0, particles_[(j - 1) * (slices_ + 1) + i].curr_pos)
          );
        }
        if(i - 1 >= 0) {
          particles_around.push_back(
            std::make_pair(1, particles_[j * (slices_ + 1) + i - 1].curr_pos)
          );
        }
        if(j + 1 <= stacks_) {
          particles_around.push_back(
            std::make_pair(2, particles_[(j + 1) * (slices_ + 1) + i].curr_pos)
          );
        }
        if(i + 1 <= slices_) {
          particles_around.push_back(
            std::make_pair(3, particles_[j * (slices_ + 1) + i + 1].curr_pos)
          );
        }
        int shared_count = 0;
        for(int k = 0; k < particles_around.size(); ++k) {
          int next = (k + 1) % particles_around.size();
          if(particles_around[next].first - particles_around[k].first == 1
            || particles_around[next].first - particles_around[k].first == -3){
            shared_count++;
            glm::vec3 v1 = particles_around[k].second - p.curr_pos;
            glm::vec3 v2 = particles_around[next].second - p.curr_pos;
            glm::vec3 vn = glm::normalize(glm::cross(v1, v2));
            if(!glm::all(glm::isnan(vn))) {
              normal_sum += vn;
            }
          }
        }
        p.normal = glm::normalize(normal_sum);
        p.shared_count = shared_count;

        // update vbo
        vbo_[vbo_idx++] = (p.curr_pos.x);
        vbo_[vbo_idx++] = (p.curr_pos.y);
        vbo_[vbo_idx++] = (p.curr_pos.z);
        vbo_[vbo_idx++] = (p.tex_coor.x);
        vbo_[vbo_idx++] = (p.tex_coor.y);
        vbo_[vbo_idx++] = (p.normal.x);
        vbo_[vbo_idx++] = (p.normal.y);
        vbo_[vbo_idx++] = (p.normal.z);
      }
    }

    is_dirty_ = false;
    return vbo_;
  };

  std::vector<uint32_t> getEBO() const {
    return ebo_;
  }

  uint32_t getIndicesCount() const {
    return ebo_.size();
  }
private:
  int slices_;
  int stacks_;
  float width_;
  float height_;
  float step_;
  std::vector<Particle> particles_;
  std::vector<float> vbo_; // row-wise
  std::vector<uint32_t> ebo_; // this should be fixed
  std::vector<std::pair<int, int>> constraint_pairs_;
  float now_ = 0;
  // for the first time generation
  bool is_dirty_ = true;
};