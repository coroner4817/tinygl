#pragma once
#include "tinygl/common.h"

struct AttrProperity
{
  int layoutId;
  int size;
};

struct VAOIDs
{
  bool isNew;
  unsigned int VAOid;
  unsigned int VBOid;
  unsigned int EBOid;
  std::vector<AttrProperity> attr_prop;
  int step;
};

struct AttrReuseProperity
{
  int layoutId;
  int index;
};

class VAOManager {

public:
  VAOManager(){}
  ~VAOManager(){}

  std::unordered_map<std::string, VAOIDs> VAOList_;

  const std::string VAOManagerToString(){
    std::string ret;
    ret += "------------VAOManagerToString\n";
    for(const auto& nv : VAOList_){
      ret += "name: " + nv.first + ", VAO: " + std::to_string(nv.second.VAOid) + "\n";
    }
    return ret;
  }

  void cleanUpVAOs(){
    // clean up resource
    if(!VAOList_.empty()) std::cout << "glDeleteVertexArrays!!!!!" << std::endl;
    for (auto vao : VAOList_)
    {
      glDeleteVertexArrays(1, &vao.second.VAOid);
      if (vao.second.isNew)
      {
        glDeleteVertexArrays(1, &vao.second.VBOid);
        glDeleteVertexArrays(1, &vao.second.EBOid);
      }
    }
  }

  void deleteVAO(const std::string& name)
  {
    if (VAOList_.find(name) == VAOList_.end())
      return;
    glDeleteVertexArrays(1, &VAOList_[name].VAOid);
    if (VAOList_[name].isNew)
    {
      glDeleteVertexArrays(1, &VAOList_[name].VBOid);
      glDeleteVertexArrays(1, &VAOList_[name].EBOid);
    }
    VAOList_.erase(name);
  }

  void useVAO(const std::string &name)
  {
    if (VAOList_.find(name) == VAOList_.end())
      return;
    glBindVertexArray(VAOList_[name].VAOid);
  }

  // for data format that is not 123123123, like 111 222 333, the position, normal and texture are separated
  // then should use glBufferSubData and when use glVertexAttribPointer should use the whole size as stride
  // https://learnopengl.com/Advanced-OpenGL/Advanced-Data

  // we can also copy buffer data using glCopyBufferSubData

  // hard coded have to input all data and properity for all attribute
  // the buffer format is 123123123123
  unsigned int generateNewVAOFromArray(std::string name, const void *vertices, int vertices_sz, const std::vector<AttrProperity> &attrprop, const void *indices=nullptr, int indices_sz=0)
  {
    unsigned int VBO, VAO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    // bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices_sz, vertices, GL_STATIC_DRAW);
    if(indices){
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
      glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices_sz, indices, GL_STATIC_DRAW);
    }
    // can use glBufferSubData to update/insert the buffer partially
    // or use glMapBuffer instead to memcpy buffer data to the GPU memory

    int step = 0;
    for (auto &attr : attrprop)
    {
      step += attr.size;
    }
    // assume will put all the data to the buffer
    int accu = 0;
    for (auto &attr : attrprop)
    {
      glVertexAttribPointer(attr.layoutId, attr.size, GL_FLOAT, GL_FALSE, step * sizeof(float), (void *)(accu * sizeof(float)));
      glEnableVertexAttribArray(attr.layoutId);
      accu += attr.size;
    }

    VAOList_[name] = {true, VAO, VBO, EBO, attrprop, step};
    glBindVertexArray(0);
    return VAO;
  }

  unsigned int generateNewVAOFromArray(std::string name, std::string reusename, std::vector<AttrReuseProperity> attr_reuse)
  {
    if (VAOList_.find(reusename) == VAOList_.end())
      return 0;
    VAOIDs hitVAO = VAOList_[reusename];

    unsigned int VAO;
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, hitVAO.VBOid);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, hitVAO.EBOid);

    for (auto &attr : attr_reuse)
    {
      int offset = 0;
      for (int i = 0; i < attr.index; ++i)
      {
        offset += hitVAO.attr_prop[i].size;
      }
      glVertexAttribPointer(attr.layoutId, hitVAO.attr_prop[attr.index].size, GL_FLOAT, GL_FALSE, hitVAO.step * sizeof(float), (void *)(offset * sizeof(float)));
      glEnableVertexAttribArray(attr.layoutId);
    }

    VAOList_[name] = {false, VAO};
    glBindVertexArray(0);
    return VAO;
  }
};