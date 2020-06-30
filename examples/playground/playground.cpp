#define STB_IMAGE_IMPLEMENTATION
#include "tinygl/stb_image.h"
#include <tinygl/utils.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <tinygl/shader_s.h>
#include <iostream>
#include <cmath>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/ext.hpp>


void calculateLightReflection();

int main()
{
  calculateLightReflection();
}

void calculateLightReflection()
{
  // interview question
  // if we know a mirror plane's normal and a input light direction
  // what is the light output direction

  glm::vec3 norm = glm::normalize(glm::vec3(1, 1, 1));
  glm::vec3 input = glm::normalize(glm::vec3(-0.526, -0.68, -0.1115));

  {
    // solution 0
    glm::vec3 output = glm::reflect(input, norm);
    std::cout << glm::to_string(output) << std::endl;
    std::cout << glm::angle(output, norm) + glm::angle(input, norm) << std::endl;
  }

  {
    // solution 1
    glm::vec3 input_backward = -input;
    float angle = glm::angle(input_backward, norm);
    glm::vec3 output = glm::rotate(input_backward, angle*2, glm::normalize(glm::cross(input_backward, norm)));
    std::cout << glm::to_string(output) << std::endl;
    std::cout << glm::angle(output, norm) + glm::angle(input, norm) << std::endl;
  }

  {
    // solution 2
    glm::vec3 output = input - 2 * (glm::dot(norm, input) / glm::length(norm)) * glm::normalize(norm);
    output = glm::normalize(output);
    std::cout << glm::to_string(output) << std::endl;
    std::cout << glm::angle(output, norm) + glm::angle(input, norm) << std::endl;
  }

  {
    // solution 3
    float x = glm::length(glm::cross(input, norm)) / glm::length(norm);
    float y = - glm::dot(input, norm) / glm::length(norm);
    glm::vec3 xdir = glm::normalize(glm::cross(glm::cross(norm, input), norm));
    glm::vec3 output = glm::normalize(x * xdir + y * glm::normalize(norm));
    std::cout << glm::to_string(output) << std::endl;
    std::cout << glm::angle(output, norm) + glm::angle(input, norm) << std::endl;
  }
}