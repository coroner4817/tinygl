#version 410 core

layout (location = 0) out vec4 FragColor;

layout (location = 5) in vec2 TexCoord;
layout (location = 6) in vec3 Normal;
layout (location = 7) in vec3 FragPos;

struct DirectLight {
  vec3 direction;
  vec3 ambient;
  vec3 diffuse;
  vec3 specular;
};

struct PointLight {
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

uniform vec3 cameraPos;
uniform DirectLight directlight;
#define NR_POINT_LIGHTS 4
uniform PointLight pointlights[NR_POINT_LIGHTS];
uniform SpotLight spotlight;

uniform sampler2D texture_diffuse0;
uniform sampler2D texture_specular0;
uniform sampler2D texture_normal0;
uniform float texture_shinness;

vec3 CalcDirectLight(DirectLight light, vec3 normal, vec3 viewDir)
{
  vec3 lightDir = normalize(-light.direction);
  // diffuse shading
  float diff = max(dot(normal, lightDir), 0.0);
  // specular shading
  vec3 reflectDir = reflect(-lightDir, normal);
  float spec = pow(max(dot(viewDir, reflectDir), 0.0), texture_shinness);
  // combine results
  vec3 ambient  = light.ambient  * vec3(texture(texture_diffuse0, TexCoord));
  vec3 diffuse  = light.diffuse  * diff * vec3(texture(texture_diffuse0, TexCoord));
  vec3 specular = light.specular * spec * vec3(texture(texture_specular0, TexCoord));
  return (ambient + diffuse + specular);
}

vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
  vec3 lightDir = normalize(light.position - fragPos);
  // diffuse shading
  float diff = max(dot(normal, lightDir), 0.0);
  // specular shading
  vec3 reflectDir = reflect(-lightDir, normal);
  float spec = pow(max(dot(viewDir, reflectDir), 0.0), texture_shinness);
  // attenuation
  float distance    = length(light.position - fragPos);
  float attenuation = 1.0 / (light.constant + light.linear * distance +
            light.quadratic * (distance * distance));
  // combine results
  vec3 ambient  = light.ambient  * vec3(texture(texture_diffuse0, TexCoord));
  vec3 diffuse  = light.diffuse  * diff * vec3(texture(texture_diffuse0, TexCoord));
  vec3 specular = light.specular * spec * vec3(texture(texture_specular0, TexCoord));
  ambient  *= attenuation;
  diffuse  *= attenuation;
  specular *= attenuation;
  return (ambient + diffuse + specular);
}

float calcSpotLight(SpotLight light, vec3 fragPos)
{
  float theta = abs(dot(normalize(light.position - fragPos), normalize(-light.direction)));
  float epsilon = light.innerCutoff - light.outerCutoff;
  float intensity = clamp((theta - light.outerCutoff) / epsilon, 0.0, 1.0);
  return intensity;
}

void main()
{
  vec3 norm = normalize(Normal);
  vec3 viewDir = normalize(cameraPos - FragPos);
  vec3 result = CalcDirectLight(directlight, norm, viewDir);
  float intensity = calcSpotLight(spotlight, FragPos);
  for(int i = 0; i < NR_POINT_LIGHTS; i++)
    result += CalcPointLight(pointlights[i], norm, FragPos, viewDir) * intensity;

  FragColor = vec4(result, 1.0);
}