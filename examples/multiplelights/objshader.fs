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

struct Material {
  vec3 ambient;
  sampler2D diffuse;
  sampler2D specular;
  sampler2D emission;
  float shinness;
};

uniform vec3 cameraPos;
uniform DirectLight directlight;
#define NR_POINT_LIGHTS 4
uniform PointLight pointlights[NR_POINT_LIGHTS];
uniform SpotLight spotlight;
uniform Material material;

vec3 CalcDirectLight(DirectLight light, vec3 normal, vec3 viewDir)
{
  vec3 lightDir = normalize(-light.direction);
  // diffuse shading
  float diff = max(dot(normal, lightDir), 0.0);
  // specular shading
  vec3 reflectDir = reflect(-lightDir, normal);
  float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shinness);
  // combine results
  vec3 ambient  = light.ambient  * vec3(texture(material.diffuse, TexCoord)) * material.ambient;
  vec3 diffuse  = light.diffuse  * diff * vec3(texture(material.diffuse, TexCoord));
  vec3 specular = light.specular * spec * vec3(texture(material.specular, TexCoord));
  return (ambient + diffuse + specular);
}

vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
  vec3 lightDir = normalize(light.position - fragPos);
  // diffuse shading
  float diff = max(dot(normal, lightDir), 0.0);
  // specular shading
  vec3 reflectDir = reflect(-lightDir, normal);
  float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shinness);
  // attenuation
  float distance    = length(light.position - fragPos);
  float attenuation = 1.0 / (light.constant + light.linear * distance +
            light.quadratic * (distance * distance));
  // combine results
  vec3 ambient  = light.ambient  * vec3(texture(material.diffuse, TexCoord)) * material.ambient;
  vec3 diffuse  = light.diffuse  * diff * vec3(texture(material.diffuse, TexCoord));
  // material.specular is a specular map like: https://learnopengl.com/img/textures/container2_specular.png
  // material.specular is either 0 or a value large than 0 smaller than 1, so floor(vec3(1) - 2*vec3(texture(material.specular, TexCoord))) yield 1 or 0
  // so here the first sub equation yield range of 2*vec3(texture(material.emission, TexCoord)) or 0. 2 is just amplify. Then add the original specular map
  vec3 specular = (floor(vec3(1) - vec3(texture(material.specular, TexCoord))) * vec3(texture(material.emission, TexCoord)) * 2 + vec3(texture(material.specular, TexCoord))) * spec * light.specular;
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