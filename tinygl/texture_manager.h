#pragma once

#define STB_IMAGE_IMPLEMENTATION
#include "tinygl/stb_image.h"
#include "tinygl/common.h"

struct TextureProperity
{
  unsigned int textureId;
  int activeId;
};

class TextureManager {
public:
  TextureManager(){}
  ~TextureManager(){}

  const std::string TextureManagerToString()
  {
    std::string ret;
    ret += "------------TextureManagerToString\n";
    for(const auto& ti : textureTypeActiveIDMap){
      ret += "type " + std::to_string(ti.first) + ": ";
      for(const auto& id : ti.second){
        ret += std::to_string(id) + ", ";
      }
      ret += "\n";
    }
    return ret;
  }

  std::vector<TextureProperity> texList_;
  std::set<std::string> texPathSet_;
  std::unordered_map<int, std::vector<unsigned int>> textureTypeActiveIDMap;

  unsigned int loadTexture2DBase(const std::string &path, bool gammaCorrected=false)
  {
    unsigned int texture1;
    glGenTextures(1, &texture1);
    glBindTexture(GL_TEXTURE_2D, texture1);

    // set the texture wrapping/filtering options (on the currently bound texture object)
    // use GL_CLAMP_TO_EDGE for transparent texture
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    int width, height, nrChannels;
    std::string texture_path = path;
    unsigned char *data = stbi_load(texture_path.c_str(), &width, &height, &nrChannels, 0);
    if (data)
    {
      GLenum internalFormat;
      GLenum format;
      if (nrChannels == 1){
          internalFormat = GL_RED;
          format = GL_RED;
      }
      else if (nrChannels == 3){
          internalFormat = gammaCorrected ? GL_SRGB : GL_RGB;
          format = GL_RGB;
      }
      else if (nrChannels == 4){
          internalFormat = gammaCorrected ? GL_SRGB_ALPHA : GL_RGBA;
          format = GL_RGBA;
      }

      glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, GL_UNSIGNED_BYTE, data);
      glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
      std::cout << "Failed to load texture: " << path << std::endl;
    }
    stbi_image_free(data);
    return texture1;
  }

  bool loadTexture2D(const std::string &path, int activeId, bool gammaCorrected=false)
  {
    // texture already loaded, no need to load again
    if(texPathSet_.find(path) != texPathSet_.end()) return false;

    texList_.push_back({loadTexture2DBase(path, gammaCorrected), activeId});

    texPathSet_.insert(path);
    return true;
  }

  bool loadTexture2D(const std::string &path, int activeId, int type)
  {
    bool ret = loadTexture2D(path, activeId);
    if(textureTypeActiveIDMap.find(type) == textureTypeActiveIDMap.end()){
      textureTypeActiveIDMap[type] = std::vector<unsigned int>();
    }
    textureTypeActiveIDMap[type].push_back(activeId);
    return ret;
  }

  void cleanUpTextures(){
    if(!texList_.empty()) std::cout << "glDeleteTextures!!!!!" << std::endl;
    for (auto tex : texList_){
      glDeleteTextures(1, &tex.textureId);
    }
  }

  void bindAllTextures(){
    // bind texture
    for (auto &itex : texList_)
    {
      glActiveTexture(GL_TEXTURE0 + itex.activeId);
      glBindTexture(GL_TEXTURE_2D, itex.textureId);
    }
  }
};