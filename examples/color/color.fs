#version 410 core

layout (location = 0) out vec4 FragColor;
layout (location = 4) in vec2 TexCoord;
layout (location = 5) in vec3 vertCoord;

uniform sampler2D texture1;
uniform sampler2D texture2;
uniform float radius;
uniform float mixvalue;

uniform vec3 objectColor;
uniform vec3 lightColor;

void main()
{
  if(length(vertCoord) < radius){
    FragColor = vec4(lightColor * objectColor, 1.0);
  }else{
    FragColor = texture(texture1, TexCoord);
  }
}