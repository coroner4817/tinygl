#version 410 core
layout (location = 0) out vec4 FragColor;
layout (location = 1) in vec3 FragPos;

uniform vec4 color;

void main()
{
  FragColor = color;
}