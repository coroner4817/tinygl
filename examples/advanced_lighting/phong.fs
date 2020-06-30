#version 410 core
layout (location = 0) out vec4 FragColor;

in VS_OUT {
  vec3 FragPos;
  vec3 Normal;
  vec2 TexCoords;
} fs_in;

uniform sampler2D texture1;

uniform vec3 lightPos;
uniform vec3 viewPos;
uniform bool blinn;

void main()
{
  vec3 color = texture(texture1, fs_in.TexCoords).rgb;
  // ambient
  vec3 ambient = 0.05 * color;
  // diffuse
  vec3 lightDir = normalize(lightPos - fs_in.FragPos);
  vec3 normal = normalize(fs_in.Normal);
  float diff = max(dot(lightDir, normal), 0.0);
  vec3 diffuse = diff * color;
  // specular
  vec3 viewDir = normalize(viewPos - fs_in.FragPos);
  vec3 reflectDir = reflect(-lightDir, normal);
  float spec = 0.0;
  if(blinn)
  {
    // use the normal and the halfway vec's dot product to represent how close 2 vector are
    vec3 halfwayDir = normalize(lightDir + viewDir);
    spec = pow(max(dot(normal, halfwayDir), 0.0), 32.0);
  }
  else
  {
    // use the view vector and reflect light vec dot product to represent how close they are
    // have the issue that when angle > 90, the dot product is 0 which make the specular light clip
    vec3 reflectDir = reflect(-lightDir, normal);
    spec = pow(max(dot(viewDir, reflectDir), 0.0), 8.0);
  }
  vec3 specular = vec3(0.3) * spec; // assuming bright white light color
  FragColor = vec4(ambient + diffuse + specular, 1.0);
}