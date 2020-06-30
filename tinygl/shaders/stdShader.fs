#version 410 core
layout (location = 0) out vec4 FragColor;

layout (location = 3) in vec2 TexCoords;

uniform sampler2D texture1;
uniform bool isColor; // default is false
uniform vec3 color;

void main()
{
  if(!isColor){
    FragColor = texture(texture1, TexCoords);
  }else{
    FragColor = vec4(color, 1.0);
  }
}