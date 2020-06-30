#pragma once
#include "tinygl/common.h"
#include "tinygl/shader_s.h"

// for single light space only
struct ShadowMeta {
  glm::vec3 lookat;
  glm::vec3 up;
  int shadowWidth;
  int shadowHeight;
  float fov;
  float nearPlane;
  float farPlane;

  unsigned int depthMap;
  unsigned int depthMapFBO;
  glm::mat4 lightProjection;
  glm::mat4 lightView;
  Shader* depthShader = nullptr;
};

class Light {
public:
  Light(const glm::vec3& pos=glm::vec3(), const glm::vec4& color=glm::vec4(1)){
    pos_ = pos;
    color_ = color;
  }
  ~Light(){
    if(shadowMeta_.depthShader) delete shadowMeta_.depthShader;
  }
  void setPosition(const glm::vec3& pos){
    pos_ = pos;
    if(enableShadow_){
      shadowMeta_.lightView = glm::lookAt(pos_, shadowMeta_.lookat, shadowMeta_.up);
    }
  };
  glm::vec3 getPosition() const {
    return pos_;
  }
  glm::vec4 getColor() const {
    return color_;
  }
  void setColor(const glm::vec4& color){
    color_ = color;
  }
  glm::mat4 getLightSpaceMat() const {
    return shadowMeta_.lightProjection * shadowMeta_.lightView;
  }
  unsigned int getDepthBuffer() const {
    return shadowMeta_.depthMap;
  }
  void activateDepthMap(unsigned int id)
  {
    glActiveTexture(GL_TEXTURE0 + id);
    glBindTexture(GL_TEXTURE_2D, getDepthBuffer());
  }

  void enableShadow(const ShadowMeta& meta){
    enableShadow_ = true;
    shadowMeta_ = meta;

    // configure depth map FBO
    // shadow might have a different resolution than the actual content
    // because we only care about the depth here
    glGenFramebuffers(1, &shadowMeta_.depthMapFBO);
    glGenTextures(1, &shadowMeta_.depthMap);
    glBindTexture(GL_TEXTURE_2D, shadowMeta_.depthMap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, shadowMeta_.shadowWidth, shadowMeta_.shadowHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    // use GL_CLAMP_TO_BORDER so that for the fragment outside of the light projection frustum
    // all the depth value are 1
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
    // attach depth texture as FBO's depth buffer
    glBindFramebuffer(GL_FRAMEBUFFER, shadowMeta_.depthMapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadowMeta_.depthMap, 0);
    // set the color buffer and read buffer not available
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    shadowMeta_.lightProjection = glm::perspective(glm::radians(shadowMeta_.fov),
                                  (float)shadowMeta_.shadowWidth / (float)shadowMeta_.shadowHeight,
                                  shadowMeta_.nearPlane, shadowMeta_.farPlane);
    shadowMeta_.lightView = glm::lookAt(pos_, shadowMeta_.lookat, shadowMeta_.up);

    shadowMeta_.depthShader = new Shader(COMMON_SHADERS_BASEPATH + "/shadowDepth.vs", COMMON_SHADERS_BASEPATH + "/shadowDepth.fs");
    shadowMeta_.depthShader->init("model");
    shadowMeta_.depthShader->use();
    shadowMeta_.depthShader->setMatrix4fv("lightSpaceMatrix", getLightSpaceMat());
  }

  void depthPass(std::function<void(Shader&)> renderFunc){
    shadowMeta_.depthShader->use();
    shadowMeta_.depthShader->setMatrix4fv("lightSpaceMatrix", getLightSpaceMat());
    // first pass
    // shadow map is smaller than actual content
    // ViewPort doesn't affect the content that is rendered!!!
    glViewport(0, 0, shadowMeta_.shadowWidth, shadowMeta_.shadowHeight);
    // write depth info to our FBO
    glBindFramebuffer(GL_FRAMEBUFFER, shadowMeta_.depthMapFBO);
    // don't clear GL_COLOR_BUFFER_BIT so that user doesn't see black frame while processing this pass
    glClear(GL_DEPTH_BUFFER_BIT);
    // solve the peter panning issue by using the back face to determine the depth map
    glCullFace(GL_FRONT);

    renderFunc(*shadowMeta_.depthShader);

    glCullFace(GL_BACK); // reset to default
    // bind FBO back to the default FBO for rendering
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    // after first pass, we store the frame depth map in the depthMap texture buffer
    // the depth map is in the light POV space
  }

private:
  glm::vec3 pos_;
  glm::vec4 color_;

  bool enableShadow_ = false;
  ShadowMeta shadowMeta_;
};