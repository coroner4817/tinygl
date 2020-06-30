#version 410 core
layout (location = 0) out vec4 FragColor;
layout (location = 1) in vec3 FragPos;

uniform samplerCube environmentMap;

void main()
{
  // textureLod will load the level mipmap, level 0 is the original
  vec3 envColor = textureLod(environmentMap, FragPos, 0.0).rgb;

  // HDR tonemap and gamma correct
  envColor = envColor / (envColor + vec3(1.0));
  envColor = pow(envColor, vec3(1.0/2.2));

  FragColor = vec4(envColor, 1.0);
}