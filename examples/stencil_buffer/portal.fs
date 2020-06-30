#version 410 core
layout (location = 0) out vec4 FragColor;

layout (location = 3) in vec2 TexCoords;

uniform sampler2D texture1;
uniform float radius;

void main()
{
  // useless now because the the stencil is write for all fragment
  // if(length(TexCoords - vec2(1, 1)) < radius){
  //   vec3 tex = texture(texture1, TexCoords).xyz;
  //   FragColor = vec4(tex, 0);
  // }else{
  //   FragColor = texture(texture1, TexCoords);
  // }

  vec3 tex = texture(texture1, TexCoords).xyz;
  FragColor = vec4(tex, 0);
}