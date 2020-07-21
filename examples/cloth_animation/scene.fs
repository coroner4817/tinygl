#version 410 core
layout (location = 0) out vec4 FragColor;
in VS_OUT {
  vec2 TexCoord;
  vec3 Normal;
  vec3 FragPos;
  vec4 LightSpacePos;
} fs_in;
/////////////////////////
struct DirectLight {
  vec3 direction;
  vec3 ambient;
  vec3 diffuse;
  vec3 specular;
};
uniform DirectLight directLight;
/////////////////////////
struct PointLight {
  vec3 position;
  vec3 ambient;
  vec3 diffuse;
  vec3 specular;
  float constant;
  float linear;
  float quadratic;
};
uniform PointLight pointlight;
/////////////////////////
struct Texture {
  sampler2D diffuseTexture;
  vec3 specular;
  float shinness;
};
uniform Texture material;
/////////////////////////
uniform vec3 cameraPos;
uniform sampler2D shadowMap;

float ShadowCalculation(vec4 fragPosLightSpace, vec3 lightDir)
{
  // perform perspective divide
  // normalize the light space position [-w, w] into range of [-1, 1]
  // so that can compare with the depth buffer's value with is in [0, 1]
  vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
  // convert into [0, 1], xy need to be in [0, 1] so that can use as TexCoord
  // z also need to be in [0, 1] so that we can compare
  // projCoords is in the light space
  projCoords = projCoords * 0.5 + 0.5;
  // the value store in the depth buffer is the closed to the light!!!
  // remember in the first pass we are using the light space projection matrix
  // depth is a single float store in the red section
  // projCoords.xy is in the light projected space so that we can use it to access the depth frame buffer
  float closestDepth = texture(shadowMap, projCoords.xy).r;
  float currentDepth = projCoords.z;

  // solve shadow acne issue
  // this issue is bscause the depth map have a lower resolution than the fragment resolution
  // and when project the frag pos to the light space, the black stripe part
  // is using the incorrect depth so it fail the depth test and fail to render
  // to fix this we can add a bias offset to make it always at least pass the local depth test
  float bias = max(0.05 * (1.0 - dot(normalize(fs_in.Normal), lightDir)), 0.005);
  // float shadow = currentDepth - bias > closestDepth  ? 1.0 : 0.0;

  // PCF to anti alias of shadow due to low resolution of the depth map
  float shadow = 0.0;
  vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
  // mix the 8 texel around this fragment together to generate this shadow
  for(int x = -1; x <= 1; ++x) {
    for(int y = -1; y <= 1; ++y) {
      // accumulate the shadow for each 9 texels
      float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
      shadow += currentDepth - bias > pcfDepth  ? 1.0 : 0.0;
    }
  }
  shadow /= 9.0;

  // set shadow to 0 when fragment is out of light projection frustum
  if(projCoords.z > 1.0)
    shadow = 0.0;

  // FragColor = vec4(vec3(closestDepth), 1.0);
  return shadow;
}

vec3 CalcDirectLight(DirectLight light, vec3 normal, vec3 viewDir)
{
  vec3 lightDir = normalize(-light.direction);
  // diffuse shading
  float diff = max(dot(normal, lightDir), 0.0);
  // specular shading
  vec3 halfwayDir = normalize(lightDir + viewDir);
  float spec = pow(max(dot(normal, halfwayDir), 0.0), material.shinness);
  // combine results
  vec3 ambient  = light.ambient * texture(material.diffuseTexture, fs_in.TexCoord).rgb;
  vec3 diffuse  = light.diffuse * diff * texture(material.diffuseTexture, fs_in.TexCoord).rgb;
  vec3 specular = light.specular * spec * vec3(material.specular);
  return (ambient + diffuse + specular);
}

vec3 CalcPointLight(PointLight light, vec3 normal, vec3 viewDir, vec3 fragPos)
{
  vec3 lightDir = normalize(light.position - fragPos);
  // diffuse shading
  float diff = max(dot(normal, lightDir), 0.0);
  // specular shading
  vec3 halfwayDir = normalize(lightDir + viewDir);
  float spec = pow(max(dot(normal, halfwayDir), 0.0), material.shinness);
  // attenuation
  float distance = length(light.position - fragPos);
  float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));
  // combine results
  vec3 ambient = light.ambient * texture(material.diffuseTexture, fs_in.TexCoord).rgb * attenuation;
  vec3 diffuse = light.diffuse * diff * texture(material.diffuseTexture, fs_in.TexCoord).rgb * attenuation;
  vec3 specular = light.specular * spec * vec3(material.specular) * attenuation;

  float shadow = ShadowCalculation(fs_in.LightSpacePos, lightDir);
  return (ambient + (diffuse + specular) * (1.0 - shadow));
}

void main()
{
  vec3 norm = normalize(fs_in.Normal);
  vec3 viewDir = normalize(cameraPos - fs_in.FragPos);
  vec3 directResult = CalcDirectLight(directLight, norm, viewDir);
  vec3 pointResult = CalcPointLight(pointlight, norm, viewDir, fs_in.FragPos);

  FragColor = vec4(directResult + pointResult, 1.0);
}