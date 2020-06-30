#pragma once

#include <glad/glad.h>
#include "tinygl/common.h"
#include "tinygl/camera.h"

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

class Shader : public CameraViewMatrixUpdateListener
{
public:
  unsigned int ID;
  unsigned int CreateShader(const std::string& path, unsigned int SHAER_TYPE, const std::string& name)
  {
    std::string code;
    std::ifstream shaderFile;
    shaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    try{
      shaderFile.open(path);
      std::stringstream shaderStream;
      shaderStream << shaderFile.rdbuf();
      shaderFile.close();
      code = shaderStream.str();
    }catch (std::ifstream::failure e) {
      std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ" << std::endl;
      std::cout << path << std::endl;
    }

    unsigned int shaderID;
    shaderID = glCreateShader(SHAER_TYPE);
    const char* code_c = code.c_str();
    glShaderSource(shaderID, 1, &code_c, NULL);
    glCompileShader(shaderID);
    checkCompileErrors(shaderID, name);
    return shaderID;
  }

  // constructor generates the shader on the fly
  // ------------------------------------------------------------------------
  Shader(const std::string& vertexPath, const std::string& fragmentPath, const std::string& geometryPath="")
  {
    unsigned int vertex, fragment, geometry;
    vertex = CreateShader(vertexPath, GL_VERTEX_SHADER, "VERTEX");
    fragment = CreateShader(fragmentPath, GL_FRAGMENT_SHADER, "FRAGMENT");
    if(!geometryPath.empty()){
      geometry = CreateShader(geometryPath, GL_GEOMETRY_SHADER, "GEOMETRY");
    }

    // shader Program
    ID = glCreateProgram();
    glAttachShader(ID, vertex);
    glAttachShader(ID, fragment);
    if(!geometryPath.empty()) glAttachShader(ID, geometry);
    glLinkProgram(ID);
    checkCompileErrors(ID, "PROGRAM");
    // delete the shaders as they're linked into our program now and no longer necessary
    glDeleteShader(vertex);
    glDeleteShader(fragment);
    if(!geometryPath.empty()) glDeleteShader(geometry);
  }
  // activate the shader
  // ------------------------------------------------------------------------
  void use()
  {
    glUseProgram(ID);
  }
  // utility uniform functions
  // ------------------------------------------------------------------------
  void setBool(const std::string &name, bool value) const
  {
    glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value);
  }
  // ------------------------------------------------------------------------
  void setInt(const std::string &name, int value) const
  {
    glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
  }
  // ------------------------------------------------------------------------
  void setFloat(const std::string &name, float value) const
  {
    glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
  }
  // ------------------------------------------------------------------------
  void setMatrix4fv(const std::string &name, const glm::mat4 &value)
  {
    glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, glm::value_ptr(value));
  }
  // ------------------------------------------------------------------------
  void setVec3(const std::string &name, const glm::vec3 &value)
  {
    glUniform3f(glGetUniformLocation(ID, name.c_str()), value.x, value.y, value.z);
  }
  // ------------------------------------------------------------------------
  void setVec4(const std::string &name, const glm::vec4 &value)
  {
    glUniform4f(glGetUniformLocation(ID, name.c_str()), value.x, value.y, value.z, value.w);
  }
  // ------------------------------------------------------------------------
  void setTexturesByType(int type, const std::vector<unsigned int>& ids)
  {
    if(textureTypeBasenameMap_.find(type) == textureTypeBasenameMap_.end()) return;

    int i = 0;
    for(const auto& id : ids){
      setInt(textureTypeBasenameMap_[type] + std::to_string(i++), id);
    }
  }

  void setTextureBaseName(int type, const std::string& basename)
  {
    textureTypeBasenameMap_[type] = basename;
  }

  void onViewMatrixUpdate(const glm::mat4& new_ViewMatrix, const glm::vec3& camPos)
  {
    use();
    setMatrix4fv(viewName_, new_ViewMatrix);
    if(cameraPosName_.size()){
      // or can extract the cameraPos from the view Matrix: glm::vec3(glm::inverse(viewMat)[3])
      setVec3(cameraPosName_, camPos);
    }
  }

  void setModelMatrix(const glm::vec3& pos=glm::vec3(),
                      const glm::vec3& scl=glm::vec3(1),
                      float angle=0, const glm::vec3& axis=glm::vec3())
  {
    setMatrix4fv(modelName_, GetModelMat(pos, scl, angle, axis));
  }

  void init(const std::string& model, const std::string& view="", Camera* cam=nullptr, const std::string& proj="", const glm::mat4& pmat=glm::mat4(1), const std::string& cameraPos="")
  {
    use();
    modelName_ = model;
    viewName_ = view;
    projName_ = proj;
    cameraPosName_ = cameraPos;
    if(cam) cam->addViewMatrixUpdateListener(this);
    setMatrix4fv(modelName_, glm::mat4(1.0f));
    if(proj.size()){
      setMatrix4fv(projName_, pmat);
    }
  }

  void bindUniformBlock(const std::string& name, unsigned int bindpointId)
  {
    unsigned int ubIdx = glGetUniformBlockIndex(ID, name.c_str());
    glUniformBlockBinding(ID, ubIdx, bindpointId);
  }

private:
  // utility function for checking shader compilation/linking errors.
  // ------------------------------------------------------------------------
  void checkCompileErrors(unsigned int shader, const std::string& type)
  {
    int success;
    char infoLog[1024];
    if (type != "PROGRAM")
    {
      glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
      if (!success)
      {
        glGetShaderInfoLog(shader, 1024, NULL, infoLog);
        std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n"
                  << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
      }
    }
    else
    {
      glGetProgramiv(shader, GL_LINK_STATUS, &success);
      if (!success)
      {
        glGetProgramInfoLog(shader, 1024, NULL, infoLog);
        std::cout << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n"
                  << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
      }
    }
  }

  std::unordered_map<int, std::string> textureTypeBasenameMap_;
  std::string modelName_;
  std::string viewName_;
  std::string projName_;
  std::string cameraPosName_;
};