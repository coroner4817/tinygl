#version 410 core
layout (location = 0) in vec3 aPos;
layout (location = 2) in vec2 aTexCoord;

layout (location = 4) out vec2 TexCoord;
layout (location = 5) out vec3 vertCoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
  gl_Position = projection * view * model * vec4(aPos, 1.0);
  TexCoord = aTexCoord;
  vertCoord = aPos.xyz;
}