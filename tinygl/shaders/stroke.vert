#version 410 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;

layout (location = 6) out vec3 Normal;
layout (location = 7) out vec3 FragPos;
layout (location = 8) out vec2 TexCoord; 

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
  gl_Position = projection * view * model * vec4(aPos, 1.0);
  Normal = mat3(transpose(inverse(model))) * aNormal;  
  FragPos = vec3(model * vec4(aPos, 1.0));
  TexCoord = aTexCoord;
}