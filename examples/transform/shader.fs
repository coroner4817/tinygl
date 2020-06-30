#version 410 core

layout (location = 0) out vec4 FragColor;
layout (location = 3) in vec3 ourColor;
layout (location = 4) in vec2 TexCoord;

uniform sampler2D ourTexture;

void main()
{
  FragColor = texture(ourTexture, TexCoord) * vec4(ourColor, 1.0);
}