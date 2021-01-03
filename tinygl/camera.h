#pragma once

#include "tinygl/common.h"
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/rotate_vector.hpp>

#include <vector>

// Default camera values
const float UPDATE_SPEED = 2.5f;

class CameraViewMatrixUpdateListener{
public:
  virtual void onViewMatrixUpdate(const glm::mat4& new_ViewMatrix, const glm::vec3& camPos){};
};

float lastX = 0;
float lastY = 0;
float xoffset, yoffset;
bool firstMouse = true;
bool mouse_moved = false;
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    xoffset = xpos - lastX;
    yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    // apply speed
    xoffset *= UPDATE_SPEED * 0.001;
    yoffset *= UPDATE_SPEED * 0.001;
    mouse_moved = true;
}

// An abstract camera class that processes input and calculates the corresponding Euler Angles, Vectors and Matrices for use in OpenGL
class Camera
{
public:
  Camera(){};
  Camera(const glm::vec3& pos, const glm::vec3& front, const glm::vec3& up, const glm::vec3& center)
        : cameraPos(pos), cameraFront(front), cameraUp(up), rotateCenter(center) {

  }
  ~Camera(){};

  // Returns the view matrix calculated using Euler Angles and the LookAt Matrix
  const glm::mat4& GetViewMatrix()
  {
    return viewMatrix;
  }

  const glm::vec3& GetCameraPos()
  {
    return cameraPos;
  }

  const glm::vec3& GetCameraFront()
  {
    return cameraFront;
  }

  void update(GLFWwindow *window, float a_delta)
  {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    float cameraSpeed = UPDATE_SPEED * a_delta;
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
      cameraPos += cameraSpeed * glm::normalize(cameraFront);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
      cameraPos -= cameraSpeed * glm::normalize(cameraFront);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
      cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
      cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
      cameraPos -= glm::normalize(cameraUp) * cameraSpeed;
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
      cameraPos += glm::normalize(cameraUp) * cameraSpeed;

    // update 2 axis and the cameraPos
    if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS){
      // rotate alone Z axis
      rotateCenter = {0, 0, cameraPos.z};
      cameraFront = glm::rotate(cameraFront, cameraSpeed, glm::vec3(0, 0, 1));
      cameraUp = glm::rotate(cameraUp, cameraSpeed, glm::vec3(0, 0, 1));
      cameraPos = glm::rotate(cameraPos-rotateCenter, cameraSpeed, glm::vec3(0, 0, 1)) + rotateCenter;
    }
    if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS){
      // rotate alone X axis
      rotateCenter = {cameraPos.x, 0, 0};
      cameraFront = glm::rotate(cameraFront, cameraSpeed, glm::vec3(1, 0, 0));
      cameraUp = glm::rotate(cameraUp, cameraSpeed, glm::vec3(1, 0, 0));
      cameraPos = glm::rotate(cameraPos-rotateCenter, cameraSpeed, glm::vec3(1, 0, 0)) + rotateCenter;
    }
    if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS){
      // rotate alone Y axis
      rotateCenter = {0, cameraPos.y, 0};
      cameraFront = glm::rotate(cameraFront, cameraSpeed, glm::vec3(0, 1, 0));
      cameraUp = glm::rotate(cameraUp, cameraSpeed, glm::vec3(0, 1, 0));
      cameraPos = glm::rotate(cameraPos-rotateCenter, cameraSpeed, glm::vec3(0, 1, 0)) + rotateCenter;
    }
    if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS){
      // set to Z axis view
      cameraPos   = glm::vec3(0.0f, 0.0f,  3.0f);
      cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
      cameraUp    = glm::vec3(0.0f, 1.0f,  0.0f);
    }
    if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS){
      // set to X axis view
      cameraPos   = glm::vec3(3.0f, 0.0f,  0.0f);
      cameraFront = glm::vec3(-1.0f, 0.0f, 0.0f);
      cameraUp    = glm::vec3(0.0f, 1.0f,  0.0f);
    }
    if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS){
      // set to Y axis view
      cameraPos   = glm::vec3(0.0f, 3.0f,  0.0f);
      cameraFront = glm::vec3(0.0f, -1.0f, 0.0f);
      cameraUp    = glm::vec3(1.0f, 0.0f,  0.0f);
    }

    if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS){
      // reset to Z view
      cameraPos   = glm::vec3(0.0f, 0.0f,  3.0f);
      cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
      cameraUp    = glm::vec3(0.0f, 1.0f,  0.0f);
    }

    if(mouse_moved){
      mouse_moved = false;

      rotateCenter = {cameraPos.x, 0, 0};
      cameraFront = glm::rotate(cameraFront, xoffset, glm::vec3(1, 0, 0));
      cameraUp = glm::rotate(cameraUp, xoffset, glm::vec3(1, 0, 0));

      rotateCenter = {0, cameraPos.y, 0};
      cameraFront = glm::rotate(cameraFront, yoffset, glm::vec3(0, 1, 0));
      cameraUp = glm::rotate(cameraUp, yoffset, glm::vec3(0, 1, 0));
    }

    viewMatrix = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);

    for(auto& cp : ViewMatrixUpdateListeners_)
    {
      cp->onViewMatrixUpdate(viewMatrix, cameraPos);
    }
  }

  void addViewMatrixUpdateListener(CameraViewMatrixUpdateListener* cp)
  {
    ViewMatrixUpdateListeners_.push_back(cp);
  }

private:
  glm::vec3 cameraPos   = glm::vec3(0.0f, 0.0f,  3.0f);
  glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
  glm::vec3 cameraUp    = glm::vec3(0.0f, 1.0f,  0.0f);
  glm::vec3 cameraRight = glm::normalize(glm::cross(cameraUp, cameraFront));
  glm::vec3 rotateCenter = glm::vec3();

  glm::mat4 viewMatrix;

  std::vector<CameraViewMatrixUpdateListener*> ViewMatrixUpdateListeners_;
};