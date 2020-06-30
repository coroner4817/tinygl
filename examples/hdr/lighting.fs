#version 410 core

layout (location = 0) out vec4 FragColor;

layout (location = 5) in vec2 TexCoord;
layout (location = 6) in vec3 Normal;
layout (location = 7) in vec3 FragPos;

#define NR_POINT_LIGHTS 4
struct Light {
  vec3 position;
  vec3 color;
};
uniform Light lights[NR_POINT_LIGHTS];

uniform vec3 cameraPos;
uniform sampler2D diffuseMap;

void main()
{
  vec3 normal = normalize(Normal);
  // get diffuse color
  vec3 diffuseColor = texture(diffuseMap, TexCoord).rgb;
  // ambient
  vec3 ambient = 0.1 * diffuseColor;

  vec3 mixture = vec3(0);
  for(int i = 0; i < NR_POINT_LIGHTS; i++){
    // diffuse
    vec3 lightDir = normalize(lights[i].position - FragPos);
    float diff = max(dot(lightDir, normal), 0.0);
    vec3 diffuse = diff * diffuseColor * lights[i].color;
    // attenuation
    float distance = length(FragPos - lights[i].position);
    diffuse *= 1.0 / (distance * distance);
    mixture += diffuse;
  }

  FragColor = vec4(ambient + mixture, 1.0);
}