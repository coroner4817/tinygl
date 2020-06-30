#version 410 core
layout (location = 0) out vec4 FragColor;

layout (location = 3) in vec2 TexCoords;

void main()
{
  FragColor = vec4(0.04, 0.28, 0.26, 1.0);
}