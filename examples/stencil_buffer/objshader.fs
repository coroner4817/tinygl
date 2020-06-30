#version 410 core
layout (location = 0) out vec4 FragColor;

layout (location = 3) in vec2 TexCoords;

uniform sampler2D texture1;
uniform vec4 color;

void main()
{
  FragColor = texture(texture1, TexCoords) * color;
}