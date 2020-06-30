#version 410 core

layout (location = 0) out vec4 FragColor;
layout (location = 9) in vec2 TexCoords;

uniform sampler2D texture_diffuse0;

void main()
{
  FragColor = texture(texture_diffuse0, TexCoords);
}
