#version 410 core
layout (location = 0) out vec4 FragColor;

layout (location = 3) in vec2 TexCoords;

uniform sampler2D texture1;

void main()
{
  vec4 texColor = texture(texture1, TexCoords);
  if(texColor.a < 0.1)
      discard;
  FragColor = texColor;
}