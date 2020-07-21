#version 410 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;
layout (location = 2) in vec3 aNormal;

out VS_OUT {
  vec2 TexCoord;
  vec3 Normal;
  vec3 FragPos;
  vec4 LightSpacePos;
} vs_out;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 lightSpaceMatrix;

void main()
{
  gl_Position = projection * view * model * vec4(aPos, 1.0);
  vs_out.TexCoord = aTexCoord;
  vs_out.Normal = mat3(transpose(inverse(model))) * aNormal;
  vs_out.FragPos = vec3(model * vec4(aPos, 1.0));

  vs_out.LightSpacePos = lightSpaceMatrix * vec4(vs_out.FragPos, 1.0);
}