#version 410 core

layout (location = 0) out vec4 FragColor;
layout (location = 3) in vec3 ourColor;
layout (location = 4) in vec2 TexCoord;
layout (location = 5) in vec2 vertCoord;

uniform sampler2D texture1;
uniform sampler2D texture2;
uniform float radius;
uniform float mixvalue;

void main()
{
  if(length(vertCoord) < radius){
    FragColor = mix(texture(texture1, TexCoord), texture(texture2, vec2(TexCoord.x, 1-TexCoord.y)), mixvalue);
  }else{
    FragColor = texture(texture1, TexCoord);
  }
}