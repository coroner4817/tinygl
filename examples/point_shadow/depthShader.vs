#version 410 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoords;
layout (location = 2) in vec3 aNormal;

uniform mat4 model;

void main()
{
  // don't apply the light space matrix here, will apply in the geometry shader
  gl_Position = model * vec4(aPos, 1.0);
}