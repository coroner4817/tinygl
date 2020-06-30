#pragma once

#include "tinygl/common.h"

// sad YINGNAN is not working with kalman
// #define YINGNAN
// #define RESAMPLE_THEN_KALMAN

#include <Eigen/Core>
#include <Eigen/Dense>
const static size_t KMstartIndex = 5;
const static float KMinitCovar = 1;
const static float KMinitNoise = 2;

struct StrokeVertex
{
  glm::vec3 Position;
  glm::vec3 Normal;
  glm::vec2 TexCoords;
};

struct StrokeDataPoint
{
  glm::vec3 pos;
  float thickness;
};

class Stroke : public TextureManager, public VAOManager {
public:
  Stroke(){
    // default parameters
    sides_ = 5;
    sides_1 = sides_ + 1;
  }

  ~Stroke(){
    cleanUpVAOs();
    cleanUpTextures();
  }

  void setSides(size_t s)
  {
    sides_ = s;
    if(sides_ == 0 || sides_ == 2){
      sides_ = 1;
    }
    sides_1 = sides_ + 1;
  }

  void setExtrudeLength(float distance)
  {
    extrudeLength = distance;
  }

#ifdef RESAMPLE_THEN_KALMAN
  void addPoint(const StrokeDataPoint& p)
  {
    // FIXME: when kalman and resample together vertices are wrong
    // might because the kalman filter smooth mess the thing up
    // prefer kalman first then resampling
    std::vector<StrokeDataPoint> resampleOut{p};
    if(extrudeLength > 0 && !vPoints_.empty()){
      if(!lineBreaks_.empty() && lineBreaks_.back() == vPoints_.size()) goto resample_finish;
      // resampleing
      if(glm::distance(p.pos, vPoints_.back().pos) > extrudeLength){
        resampleOut.clear();
        for(int i = 0; i < (int)(glm::distance(p.pos, vPoints_.back().pos) / extrudeLength); ++i){
          resampleOut.push_back({glm::normalize(p.pos - vPoints_.back().pos) * extrudeLength * (i + 1) + vPoints_.back().pos, p.thickness});
        }
      }else{
        return;
      }
    }

resample_finish:
    for(auto& p_ : resampleOut){
      StrokeDataPoint p_original = p_;
      if(enableKalman_){
        if(vPoints_.empty() || (!lineBreaks_.empty() && lineBreaks_.back() == vPoints_.size())){
          initKalman(p_.pos);
        }
        processKalman(p_.pos);
      }

      if(KMepoch > KMstartIndex){
        vPoints_.push_back(p_);
      }else{
        vPoints_.push_back(p_original);
      }
    }
    update();
  }
#else
  void addPoint(const StrokeDataPoint& p)
  {
    StrokeDataPoint p_ = p;
    if(enableKalman_){
      if(vPoints_.empty() || (!lineBreaks_.empty() && lineBreaks_.back() == vPoints_.size())){
        initKalman(p_.pos);
      }
      processKalman(p_.pos);
    }

    if(KMepoch < KMstartIndex){
      p_ = p;
    }

    std::vector<StrokeDataPoint> resampleOut{p_};
    if(extrudeLength > 0 && !vPoints_.empty()){
      if(!lineBreaks_.empty() && lineBreaks_.back() == vPoints_.size()) goto resample_finish;
      // resampleing
      if(glm::distance(p_.pos, vPoints_.back().pos) > extrudeLength){
        resampleOut.clear();
        for(int i = 0; i < (int)(glm::distance(p_.pos, vPoints_.back().pos) / extrudeLength); ++i){
          resampleOut.push_back({glm::normalize(p_.pos - vPoints_.back().pos) * extrudeLength * (i + 1) + vPoints_.back().pos, p_.thickness});
        }
      }else{
        return;
      }
    }

resample_finish:
    vPoints_.insert(vPoints_.end(), resampleOut.begin(), resampleOut.end());
    update();
  }
#endif


  void addLineBreak()
  {
    lineBreaks_.push_back(vPoints_.size());
    update();
  }

  int getVerticesCount()
  {
    return vertCount_;
  }

  void use()
  {
    // use VAO and bind texture
    useVAO(name_);
    bindAllTextures();
  }

  void enableKalman(bool b)
  {
    enableKalman_ = b;
  }

  void initKalman(const glm::vec3& k0)
  {
    KMepoch = 0;
    KMstateVec_ << k0.x, k0.y, k0.z, 0, 0, 0;
    Eigen::VectorXf covarDiag(6);
    covarDiag << 0, 0, 0, KMinitCovar, KMinitCovar, KMinitCovar;
    KMcovarMat_ = covarDiag.asDiagonal();
    KMtransMat_ = Eigen::MatrixXf::Identity(6, 6);
    KMtransMat_(0, 3) = 1;
    KMtransMat_(1, 4) = 1;
    KMtransMat_(2, 5) = 1;
    // here only observe the pos, can also pass the sensor speed and observe that
    KMobserMat_ << Eigen::Matrix3f::Identity(), Eigen::Matrix3f::Zero();
    KMnoiseMat_ = Eigen::MatrixXf::Identity(6, 6) * KMinitNoise;
    KMposStateNoise_.clear();
    KMvelStateNoise_.clear();
  }

  void processKalman(glm::vec3& zk)
  {
    Eigen::Matrix<float, 3, 1> KMmeasure;
    KMmeasure << zk.x, zk.y, zk.z;

    // time update
    auto KMstateVecOld_ = KMstateVec_;
    auto KMcovarMatOld_ = KMcovarMat_;
    KMstateVec_ = KMtransMat_ * KMstateVec_;
    KMcovarMat_ = KMtransMat_ * KMcovarMat_ * KMtransMat_.transpose() + KMnoiseMat_;
    Eigen::Matrix<float, 6, 1> KMsmooth = KMstateVecOld_ + KMcovarMatOld_ * (KMstateVec_ - KMtransMat_ * KMstateVecOld_);

    // smooth will make a lot of the cross later to be nan
    glm::vec3 ret1 = {KMsmooth(0, 0), KMsmooth(1, 0), KMsmooth(2, 0)};
    // glm::vec3 ret2 = {KMstateVec_(0, 0), KMstateVec_(1, 0), KMstateVec_(2, 0)};
    // std::cout << glm::to_string(zk) << ", " << glm::to_string(ret1) << ", " << glm::to_string(ret2) << std::endl;
    zk = ret1;

    // measurement update
    Eigen::Matrix<float, 6, 3> KMkalmanGain_ = KMcovarMat_ * KMobserMat_.transpose() * (KMobserMat_ * KMcovarMat_ * KMobserMat_.transpose()).inverse();
    Eigen::Matrix<float, 6, 1> KMstateNoise = KMkalmanGain_ * (KMmeasure - KMobserMat_ * KMstateVec_);
    KMstateVec_ = KMstateVec_ + KMstateNoise;
    KMcovarMat_ = KMcovarMat_ - KMkalmanGain_ * KMobserMat_ * KMcovarMat_;

    KMposStateNoise_.push_back(KMstateNoise(0, 0));
    KMposStateNoise_.push_back(KMstateNoise(1, 0));
    KMposStateNoise_.push_back(KMstateNoise(2, 0));
    KMvelStateNoise_.push_back(KMstateNoise(3, 0));
    KMvelStateNoise_.push_back(KMstateNoise(4, 0));
    KMvelStateNoise_.push_back(KMstateNoise(5, 0));
    if(KMepoch > 1){
      float posNoiseVar = CalcVar(KMposStateNoise_);
      float velNoiseVar = CalcVar(KMvelStateNoise_);
      KMnoiseMat_(0, 0) = posNoiseVar;
      KMnoiseMat_(1, 1) = posNoiseVar;
      KMnoiseMat_(2, 2) = posNoiseVar;
      KMnoiseMat_(3, 3) = velNoiseVar;
      KMnoiseMat_(4, 4) = velNoiseVar;
      KMnoiseMat_(5, 5) = velNoiseVar;
    }

    KMepoch++;
  }

  float CalcVar(std::vector<float> in)
  {
      float sum=0;
      for(int i=0;i<in.size();i++){
        sum=sum+in[i];
      }

      float e = sum/(float)in.size();
      sum=0;

      for(int i=0;i<in.size();i++){
        sum=sum+(in[i]-e)*(in[i]-e);
      }

      e=sum/(float)(in.size()-1);
      return e;
  }

  void KMdebug()
  {
    std::cout << "KMstateVec_: " << std::endl;
    std::cout << KMstateVec_ << std::endl;
    std::cout << "KMcovarMat_: " << std::endl;
    std::cout << KMcovarMat_ << std::endl;
    std::cout << "KMtransMat_: " << std::endl;
    std::cout << KMtransMat_ << std::endl;
    std::cout << "KMobserMat_: " << std::endl;
    std::cout << KMobserMat_ << std::endl;
    std::cout << "KMnoiseMat_: " << std::endl;
    std::cout << KMnoiseMat_ << std::endl;
  }

private:
  size_t sides_;
  size_t sides_1;

  std::vector<StrokeDataPoint> vPoints_;
  std::vector<int> lineBreaks_;

  std::string name_ = "stroke";
  int vertCount_ = 0;

  bool enableKalman_ = false;
  float extrudeLength = -1;

  int KMepoch = 0;
  Eigen::Matrix<float, 6, 1> KMstateVec_;
  Eigen::Matrix<float, 6, 6> KMcovarMat_;
  Eigen::Matrix<float, 6, 6> KMtransMat_;
  Eigen::Matrix<float, 3, 6> KMobserMat_;
  Eigen::Matrix<float, 6, 6> KMnoiseMat_;
  std::vector<float> KMposStateNoise_;
  std::vector<float> KMvelStateNoise_;

  // regenerate VAO
  void update()
  {
    if(vPoints_.size() < 3) return;
    std::vector<StrokeVertex> tDatas_;
    std::vector<unsigned int> tIndices_;

    tDatas_.reserve(sides_1 * vPoints_.size());
    tIndices_.reserve(sides_ * 6 * (vPoints_.size() - 1));

    for(int i = 0; i < vPoints_.size(); ++i){
      if(i == 0 || std::find(lineBreaks_.begin(), lineBreaks_.end(), i) != lineBreaks_.end()){
        float r = 0.5f * vPoints_[i].thickness;
        glm::vec3 dir = glm::normalize(vPoints_[i+1].pos - vPoints_[i].pos);
        glm::vec3 ax1, ax2;
        glm::vec3 perp_choice[2] = { glm::vec3(dir.z , dir.z, -dir.x - dir.y) , glm::vec3(-dir.y - dir.z, dir.x, dir.x) };
        int index = (dir.z == 0) && (-dir.x == dir.y);
        ax1 = perp_choice[index];
        ax2 = glm::normalize(glm::cross(ax1, dir));

        for(int j = 0; j < sides_1; ++j){
          float angle = j * 2 * M_PI / sides_;
          glm::vec3 p = vPoints_[i].pos + r * (ax1 * glm::cos(angle) + ax2 * glm::sin(angle));
          tDatas_.push_back({
            p,
            glm::normalize(p - vPoints_[i].pos), // for a smooth edge
            glm::vec2((float)j/(sides_), 0)
          });
        }
      }else if(i == vPoints_.size() - 1 || std::find(lineBreaks_.begin(), lineBreaks_.end(), i+1) != lineBreaks_.end()){
        for(int j = 0; j < sides_1; ++j){
          int index = i * sides_1 + j;
          tDatas_.push_back({
            vPoints_[i].pos,
            tDatas_[(i-1)*sides_1 + j].Normal,
            // with fliped UVs
            ((index / sides_1) % 2 == 1) ? glm::vec2(tDatas_[index - sides_1].TexCoords.x, 1) : glm::vec2(tDatas_[index - sides_1].TexCoords.x, 0)
          });
        }
      }else{
#ifdef YINGNAN
        glm::vec3 dir1 = glm::normalize(vPoints_[i-1].pos - vPoints_[i].pos);
        glm::vec3 dir2 = glm::normalize(vPoints_[i+1].pos - vPoints_[i].pos);
        glm::vec3 mid_dir1 = glm::normalize(dir1 + dir2);
        glm::vec3 mid_dir2 = glm::cross(dir1, dir2);
        if(glm::all(glm::isnan(mid_dir2))) mid_dir2 = {0.001, 0.001, 0.001};
        glm::vec3 plane_norm = glm::cross(mid_dir1, mid_dir2);
        if(glm::all(glm::isnan(plane_norm))) plane_norm = {0.001, 0.001, 0.001};

        for(int j=0;j<sides_1;++j){
          glm::vec3 planev_norm = glm::cross(vPoints_[i].pos-vPoints_[i-1].pos, tDatas_[(i-1)*sides_1 + j].Position-vPoints_[i].pos);
          if(glm::all(glm::isnan(planev_norm))) planev_norm = {0.001, 0.001, 0.001};
          glm::vec3 v3_dir = glm::normalize(glm::cross(plane_norm, planev_norm));
          if(glm::all(glm::isnan(v3_dir))) v3_dir = {0.001, 0.001, 0.001};
          float th = glm::angle(v3_dir, dir1);
          float scalar = vPoints_[i].thickness / 2 / glm::sin(th);
          // std::clamp(scalar, 0.f, 2 * vPoints_[i].thickness); // c++ 17
          if(scalar < 0) scalar = 0;
          if(scalar > 2 * vPoints_[i].thickness) scalar = 2 * vPoints_[i].thickness;
          glm::vec3 p = vPoints_[i].pos + v3_dir * scalar;

          // generate VAO
          int index = i * sides_1 + j;
          tDatas_.push_back({
            p, glm::normalize(p - vPoints_[i].pos),
            // with fliped UVs
            ((index / sides_1) % 2 == 1) ? glm::vec2(tDatas_[index - sides_1].TexCoords.x, 1) : glm::vec2(tDatas_[index - sides_1].TexCoords.x, 0)
          });
        }
#else
        glm::vec3 delta = vPoints_[i].pos - vPoints_[i-1].pos;
        glm::vec3 next = vPoints_[i+1].pos - vPoints_[i].pos;

        // find the angle between the 2 vectors
        glm::vec3 dir1 = glm::normalize(delta);
        glm::vec3 dir2 = glm::normalize(next);
        float angle = glm::angle(dir1, dir2);
        glm::mat4 T;
        if (angle != 0){
          glm::vec3 Axis = glm::normalize(glm::cross(dir1, dir2));
          if(glm::all(glm::isnan(Axis))) Axis = {0.001, 0.001, 0.001};
          // glm transform back to front matrices
          T = glm::translate(T, vPoints_[i].pos); //move back
          T = glm::rotate(T, angle, Axis);
          float scale = vPoints_[i].thickness / vPoints_[i-1].thickness;
          T = glm::scale(T, glm::vec3(scale)); //scale by the thickness
          T = glm::translate(T, -vPoints_[i].pos); //move to zero
        }

        for(int j=0;j<sides_1;++j){
          glm::vec4 p(tDatas_[(i-1)*sides_1 + j].Position + delta, 1.f);
          p = T * p;
          glm::vec3 q(p);
          // generate VAO
          int index = i * sides_1 + j;
          tDatas_.push_back({
            q, glm::normalize(q - vPoints_[i].pos),
            // with fliped UVs
            ((index / sides_1) % 2 == 1) ? glm::vec2(tDatas_[index - sides_1].TexCoords.x, 1) : glm::vec2(tDatas_[index - sides_1].TexCoords.x, 0)
          });
        }
#endif
      }
    }

    // generate EBO
    size_t lineStart = UINT32_MAX;
    auto nextLineIndex = lineBreaks_.begin();
    if (nextLineIndex != lineBreaks_.end()) {
      lineStart = *nextLineIndex++;
    }

    for(uint32_t i = 0; i < vPoints_.size() - 1; ++i){
      if (i != lineStart - 1){
        int idx = sides_1 * i;
        for(int j = 0; j < sides_; ++j){
          int id0 = idx + j;
          int id1 = idx + (j + 1) % sides_1;
          int id2 = id0 + sides_1;
          int id3 = id1 + sides_1;

          //triangle 0
          tIndices_.push_back(id0);tIndices_.push_back(id2);tIndices_.push_back(id1);
          //triangle 1
          tIndices_.push_back(id2);tIndices_.push_back(id3);tIndices_.push_back(id1);
        }
      }
      else  {
        if(nextLineIndex != lineBreaks_.end()) lineStart = *nextLineIndex++;
        else lineStart = UINT32_MAX;
      }
    }

    if(!tDatas_.empty() && !tIndices_.empty()){
      // delete the old VAO
      deleteVAO(name_);
      // generate new VAO use &tDatas_[0] &tIndices_[0]
      generateNewVAOFromArray(name_, &tDatas_[0], tDatas_.size() * sizeof(StrokeVertex), {{0, 3}, {1, 3}, {2, 2}},
                              &tIndices_[0], tIndices_.size() * sizeof(unsigned int));
      vertCount_ = tIndices_.size();
    }
  }
};