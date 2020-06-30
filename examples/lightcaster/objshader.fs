#version 410 core

layout (location = 0) out vec4 FragColor;

layout (location = 5) in vec2 TexCoord;
layout (location = 6) in vec3 Normal;
layout (location = 7) in vec3 FragPos;

struct Light {
  // vec3 direction;
  vec3 position;

  vec3 ambient;
  vec3 diffuse;
  vec3 specular;

  float constant;
  float linear;
  float quadratic;
};

struct SpotLight {
  vec3 position;
  vec3 direction;
  float innerCutoff;
  float outerCutoff;
};

struct Material {
  vec3 ambient;
  sampler2D diffuse;
  sampler2D specular;
  sampler2D emission;
  float shinness;
};

uniform Light light;
uniform SpotLight spotlight;
uniform Material material;
uniform vec3 cameraPos;

void main()
{
  float distance    = length(light.position - FragPos);
  float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance)); 

  // ambient
  vec3 ambient = vec3(texture(material.diffuse, TexCoord)) * light.ambient * material.ambient;

  // diffuse
  vec3 norm = normalize(Normal);
  // use (light - frag) because we want to know how much the frag expose to the light source
  vec3 lightDir = normalize(light.position - FragPos);
  // here we assume the less the angle the higher the reflection 
  // so we are using dot product which is cos(t) 
  float diff = max(dot(norm, lightDir), 0.0);
  vec3 diffuse = vec3(texture(material.diffuse, TexCoord)) * diff * light.diffuse;

  // specular
  vec3 viewDir = normalize(cameraPos - FragPos);
  vec3 reflectDir = reflect(-lightDir, norm);
  // use dot so we are calc the cos(t), the bigger the angle, the less specular reflection
  float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shinness);
  vec3 specular = (floor(vec3(1) - 2*vec3(texture(material.specular, TexCoord))) * vec3(texture(material.emission, TexCoord)) * 2 + vec3(texture(material.specular, TexCoord))) * spec * light.specular;

  // Spotlight
  float theta = abs(dot(normalize(spotlight.position - FragPos), normalize(-spotlight.direction)));
  float epsilon = spotlight.innerCutoff - spotlight.outerCutoff;
  float intensity = clamp((theta - spotlight.outerCutoff) / epsilon, 0.0, 1.0);

  // fuse the color together
  // use dot because the obj color is try to absorb the light color and only reflect the diff part of the color
  // leave the ambient from spotlight so that outer area still visible
  vec3 result = ambient * attenuation + (diffuse + specular) * attenuation * intensity;  
  FragColor = vec4(result, 1.0);
}