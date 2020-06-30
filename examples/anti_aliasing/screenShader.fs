#version 410 core
layout (location = 0) out vec4 FragColor;

layout (location = 2) in vec2 TexCoords;

uniform sampler2D screenTexture;

void main()
{
  FragColor = vec4(texture(screenTexture, TexCoords).rgb, 1.0);
}