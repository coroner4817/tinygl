#version 410 core
layout (location = 0) out vec4 color;
layout (location = 1) in vec2 TexCoords;

uniform sampler2D image;
uniform vec4 spriteColor;

void main()
{
  color = spriteColor * texture(image, TexCoords);
}