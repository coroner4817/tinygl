#version 410 core
layout (location = 0) out vec4 FragColor;

in VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
    vec4 FragPosLightSpace;
} fs_in;

uniform sampler2D diffuseTexture;
// the depth buffer obtained in the first pass
uniform sampler2D shadowMap;

uniform vec3 lightPos;
uniform vec3 cameraPos;

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
  vec3 lightDir = normalize(lightPos - fs_in.FragPos);
  float bias = max(0.05 * (1.0 - dot(normalize(fs_in.Normal), lightDir)), 0.005);
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

  return shadow;
}

void main()
{
  vec3 color = texture(diffuseTexture, fs_in.TexCoords).rgb;
  vec3 normal = normalize(fs_in.Normal);
  vec3 lightColor = vec3(1.0);
  // ambient
  vec3 ambient = 0.15 * color;
  // diffuse
  vec3 lightDir = normalize(lightPos - fs_in.FragPos);
  float diff = max(dot(lightDir, normal), 0.0);
  vec3 diffuse = diff * lightColor;
  // specular
  vec3 viewDir = normalize(cameraPos - fs_in.FragPos);
  float spec = 0.0;
  // blinn-phong
  vec3 halfwayDir = normalize(lightDir + viewDir);
  spec = pow(max(dot(normal, halfwayDir), 0.0), 64.0);
  vec3 specular = spec * lightColor;
  // calculate shadow
  float shadow = ShadowCalculation(fs_in.FragPosLightSpace);
  // since shadow is not fully black so we leave the ambient from the shadow effect
  // need to inverse the shadow coefficient, check the calc above
  vec3 lighting = (ambient + (1.0 - shadow) * (diffuse + specular)) * color;

  FragColor = vec4(lighting, 1.0);
}