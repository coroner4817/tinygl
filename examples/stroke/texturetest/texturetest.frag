#version 410 core

layout (location = 0) out vec4 FragColor;
layout (location = 2) in vec2 TexCoord;

uniform sampler2D ourTexture;

void main()
{
  FragColor = texture(ourTexture, TexCoord);
}