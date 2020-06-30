#version 410 core

layout (location = 0) out vec4 FragColor;

layout (location = 6) in vec3 Normal;
layout (location = 7) in vec3 FragPos;
layout (location = 8) in vec2 TexCoord;

uniform vec3 cameraPos;

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

uniform DirectLight directlight;
#define NR_POINT_LIGHTS 4
uniform PointLight pointlights[NR_POINT_LIGHTS];

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

void main()
{
  // vec3 norm = normalize(Normal);
  // vec3 viewDir = normalize(cameraPos - FragPos);
  // vec3 result = CalcDirectLight(directlight, norm, viewDir);
  // for(int i = 0; i < NR_POINT_LIGHTS; i++)
  //   result += CalcPointLight(pointlights[i], norm, FragPos, viewDir);

  // FragColor = vec4(result, 1.0);

  FragColor = texture(texture_diffuse0, TexCoord);
}