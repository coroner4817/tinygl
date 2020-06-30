#pragma once
#include "tinygl/common.h"

class UBOManager {

public:
  UBOManager(){}
  ~UBOManager(){}

  void generateUBO(const std::string& name, unsigned int bindPointID, unsigned int size)
  {
    unsigned int uboMatrices;
    glGenBuffers(1, &uboMatrices);
    glBindBuffer(GL_UNIFORM_BUFFER, uboMatrices);
    glBufferData(GL_UNIFORM_BUFFER, size, NULL, GL_STATIC_DRAW);
    // define the range of the buffer that links to a uniform binding point
    // bind the ubo to the global memory which can be accessed by all the shaders
    // here the bindPointID need to match the index in the glUniformBlockBinding!
    glBindBufferRange(GL_UNIFORM_BUFFER, bindPointID, uboMatrices, 0, size);

    UBOmap_[name] = uboMatrices;
  }

  void setUBO(const std::string& name, unsigned int offset, unsigned int size, const void* data)
  {
    if(UBOmap_.count(name)){
      glBindBuffer(GL_UNIFORM_BUFFER, UBOmap_[name]);
      // use glBufferSubData to update partial data
      glBufferSubData(GL_UNIFORM_BUFFER, offset, size, data);
    }
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
  }

private:
  std::unordered_map<std::string, unsigned int> UBOmap_;
};