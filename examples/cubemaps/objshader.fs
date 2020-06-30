#version 410 core

layout (location = 0) out vec4 FragColor;

layout (location = 6) in vec3 Normal;
layout (location = 7) in vec3 FragPos;

uniform vec3 cameraPos;
uniform samplerCube skybox;
uniform bool isReflection;

void main()
{
  if(isReflection){
    vec3 I = normalize(FragPos - cameraPos);
    vec3 R = reflect(I, normalize(Normal));
    FragColor = vec4(texture(skybox, R).rgb, 1.0);
  }else{
    float ratio = 1.00 / 1.52;
    vec3 I = normalize(FragPos - cameraPos);
    vec3 R = refract(I, normalize(Normal), ratio);
    FragColor = vec4(texture(skybox, R).rgb, 1.0);
  }
}