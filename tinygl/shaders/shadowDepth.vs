#version 410 core
layout (location = 0) in vec3 aPos;

uniform mat4 lightSpaceMatrix;
uniform mat4 model;

void main()
{
  // transform to light space
  // lightSpaceMatrix contain a simple view matirx at the light space
  // need to pass the model here to update the depth buffer from light POV
  gl_Position = lightSpaceMatrix * model * vec4(aPos, 1.0);
}