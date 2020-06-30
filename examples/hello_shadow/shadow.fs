#version 410 core

layout (location = 0) out vec4 FragColor;

layout (location = 5) in vec2 TexCoord;
layout (location = 6) in vec3 Normal;
layout (location = 7) in vec3 FragPos;
layout (location = 8) in vec4 LightSpacePos;

struct PointLight {
  vec3 position;

  vec3 ambient;
  vec3 diffuse;
  vec3 specular;

  float constant;
  float linear;
  float quadratic;
};

struct Texture {
  sampler2D diffuseTexture;
  vec3 specular;
};

uniform vec3 cameraPos;
uniform Texture myTexture;
uniform PointLight pointlight;
uniform float texture_shinness;

uniform vec3 lightPos;
uniform sampler2D shadowMap;

uniform sampler2D texture_diffuse0;
uniform sampler2D texture_specular0;
uniform sampler2D texture_normal0;

uniform int hasTexture;

float ShadowCalculation(vec4 fragPosLightSpace)
{
  // perform perspective divide
  // normalize the light space position [-w, w] into range of [-1, 1]
  // so that can compare with the depth buffer's value with is in [0, 1]
  vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
  // convert into [0, 1], xy need to be in [0, 1] so that can use as TexCoord
  // z also need to be in [0, 1] so that we can compare`
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
  vec3 lightDir = normalize(lightPos - FragPos);
  float bias = max(0.05 * (1.0 - dot(normalize(Normal), lightDir)), 0.005);
  // float shadow = currentDepth - bias > closestDepth  ? 1.0 : 0.0;

  // PCF to anti alias of shadow due to low resolution of the depth map
  float shadow = 0.0;
  vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
  // mix the 8 texel around this fragment together to generate this shadow
  for(int x = -1; x <= 1; ++x)
  {
    for(int y = -1; y <= 1; ++y)
    {
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
  vec3 ambient;
  vec3 diffuse;
  vec3 specular;
  if(hasTexture == 1){
    ambient  = light.ambient  * texture(texture_diffuse0, TexCoord).rgb;
    diffuse  = light.diffuse  * diff * texture(texture_diffuse0, TexCoord).rgb;
    specular = light.specular * spec * texture(texture_specular0, TexCoord).rgb;
  }else{
    ambient  = light.ambient  * texture(myTexture.diffuseTexture, TexCoord).rgb;
    diffuse  = light.diffuse  * diff * texture(myTexture.diffuseTexture, TexCoord).rgb;
    specular = light.specular * spec * vec3(myTexture.specular);
  }
  ambient  *= attenuation;
  diffuse  *= attenuation;
  specular *= attenuation;

  float shadow = ShadowCalculation(LightSpacePos);
  return (ambient + (diffuse + specular) * (1.0 - shadow));
}

void main()
{
  vec3 norm = normalize(Normal);
  vec3 viewDir = normalize(cameraPos - FragPos);
  vec3 result = CalcPointLight(pointlight, norm, FragPos, viewDir);

  FragColor = vec4(result, 1.0);
}