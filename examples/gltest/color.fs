#version 410 core

layout (location = 0) out vec4 FragColor;

layout (location = 4) in vec2 TexCoord;
layout (location = 5) in vec3 vertCoord;
layout (location = 6) in vec3 Normal;
layout (location = 7) in vec3 FragPos;

uniform sampler2D texture1;
uniform sampler2D texture2;

uniform vec3 objectColor;
uniform vec3 lightColor;
uniform vec3 lampPos;
uniform vec3 cameraPos;

void main()
{
  // ambient
  float ambientStrength = 0.1;
  vec3 ambient = ambientStrength * lightColor;

  // diffuse
  vec3 norm = normalize(Normal);
  // use (light - frag) because we want to know how much the frag expose to the light source
  vec3 lightDir = normalize(lampPos - FragPos);
  // here we assume the less the angle the higher the reflection 
  // so we are using dot product which is cos(t) 
  float diff = max(dot(norm, lightDir), 0.0);
  vec3 diffuse = diff * lightColor;

  // specular
  float specularStrength = 0.8, shinness = 256;
  vec3 viewDir = normalize(cameraPos - FragPos);
  vec3 reflectDir = reflect(-lightDir, norm);
  // use dot so we are calc the cos(t), the bigger the angle, the less specular reflection
  float spec = pow(max(dot(viewDir, reflectDir), 0.0), shinness);
  vec3 specular = specularStrength * spec * lightColor;  

  // fuse the color together
  // use dot because the obj color is try to absorb the light color and only reflect the diff part of the color
  vec3 result = (ambient + diffuse + specular) * objectColor;
  FragColor = vec4(result, 1.0);
}