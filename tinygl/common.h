#pragma once

#include <tinygl/utils.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <cmath>
#include <vector>
#include <unordered_map>
#include <functional>
#include <memory>
#include <algorithm>
#include <set>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/ext.hpp>

GLenum glCheckError_(const char *file, int line)
{
    GLenum errorCode;
    while ((errorCode = glGetError()) != GL_NO_ERROR)
    {
        std::string error;
        switch (errorCode)
        {
            case GL_INVALID_ENUM:                  error = "INVALID_ENUM"; break;
            case GL_INVALID_VALUE:                 error = "INVALID_VALUE"; break;
            case GL_INVALID_OPERATION:             error = "INVALID_OPERATION"; break;
            case GL_STACK_OVERFLOW:                error = "STACK_OVERFLOW"; break;
            case GL_STACK_UNDERFLOW:               error = "STACK_UNDERFLOW"; break;
            case GL_OUT_OF_MEMORY:                 error = "OUT_OF_MEMORY"; break;
            case GL_INVALID_FRAMEBUFFER_OPERATION: error = "INVALID_FRAMEBUFFER_OPERATION"; break;
        }
        std::cout << error << " | " << file << " (" << line << ")" << std::endl;
    }
    return errorCode;
}
#define glCheckError() glCheckError_(__FILE__, __LINE__)

#define DRAW(count) glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, 0);

static const std::string ASSETS_BASEPATH  = "../assets/";
static const std::string ASSETS_2D_BASEPATH  = "../assets/2D/";
static const std::string ASSETS_3D_BASEPATH  = "../assets/3D/";
static const std::string ASSETS_PBR_BASEPATH  = "../assets/pbr/";
static const std::string ASSETS_SKYBOX_BASEPATH  = "../assets/skybox/";
static const std::string COMMON_SHADERS_BASEPATH  = "../tinygl/shaders/";

void drawLine(const glm::vec3& st, const glm::vec3& ed, const glm::vec4& color, const glm::mat4& perspective)
{
  const char *lineVertexShader = "#version 410 core\n"
    "layout (location = 0) in vec3 aPos;\n"
    "uniform mat4 perspective;"
    "void main()\n"
    "{\n"
    "   gl_Position = perspective * vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
    "}\0";
  const char *lineFragmentShader = "#version 410 core\n"
    "layout (location = 0) out vec4 FragColor;\n"
    "uniform vec4 lineColor;"
    "void main()\n"
    "{\n"
    "   FragColor = lineColor;\n"
    "}\n\0";

  unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
  unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER); // the first fragment shader that outputs the color orange
  unsigned int shaderProgram = glCreateProgram();
  glShaderSource(vertexShader, 1, &lineVertexShader, NULL);
  glCompileShader(vertexShader);
  glShaderSource(fragmentShader, 1, &lineFragmentShader, NULL);
  glCompileShader(fragmentShader);
  // link the first program object
  glAttachShader(shaderProgram, vertexShader);
  glAttachShader(shaderProgram, fragmentShader);
  glLinkProgram(shaderProgram);

  float vertics[2*3];
  vertics[0] = st.x;
  vertics[1] = st.y;
  vertics[2] = st.z;
  vertics[3] = ed.x;
  vertics[4] = ed.y;
  vertics[5] = ed.z;

  unsigned int VBO, VAO;
  glGenVertexArrays(1, &VAO); // we can also generate multiple VAOs or buffers at the same time
  glGenBuffers(1, &VBO);
  glBindVertexArray(VAO);
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertics), vertics, GL_STATIC_DRAW);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, (3) * sizeof(float), (void*)0);	// Vertex attributes stay the same
  glEnableVertexAttribArray(0);

  glUseProgram(shaderProgram);
  int perspectiveLoc = glGetUniformLocation(shaderProgram, "perspective");
  glUniformMatrix4fv(perspectiveLoc, 1, GL_FALSE, glm::value_ptr(perspective));
  int lineColorLocation = glGetUniformLocation(shaderProgram, "lineColor");
  glUniform4f(lineColorLocation, color.x, color.y, color.z, color.w);
  glBindVertexArray(VAO);
  glDrawArrays(GL_LINE_STRIP, 0, 2);
}

const glm::mat4 GetModelMat(const glm::vec3& pos,
                        const glm::vec3& scl,
                        float angle, const glm::vec3& axis)
{
  glm::mat4 ret = glm::mat4(1.0f);
  ret = glm::translate(ret, pos);
  if(angle != 0.f){
    // this is because it is multiply from right to left
    ret = glm::translate(ret, scl/2);
    ret = glm::rotate(ret, angle, axis);
    ret = glm::translate(ret, -scl/2);
  }
  ret = glm::scale(ret, scl);
  return ret;
}