#version 410 core
// FragColor woll be output to the first color attachment buffer
layout (location = 0) out vec4 FragColor;
// BrightColor will be output to the second color attachment buffer
layout (location = 1) out vec4 BrightColor;

in VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
} fs_in;

struct Light {
    vec3 Position;
    vec3 Color;
};

uniform Light lights[4];
uniform sampler2D diffuseTexture;
uniform bool isColor;
uniform vec3 color;

uniform vec3 viewPos;

void main()
{
    if(!isColor){
      vec3 color = texture(diffuseTexture, fs_in.TexCoords).rgb;
      vec3 normal = normalize(fs_in.Normal);
      // ambient
      vec3 ambient = 0.1 * color;
      // lighting
      vec3 lighting = vec3(0.0);
      vec3 viewDir = normalize(viewPos - fs_in.FragPos);
      for(int i = 0; i < 4; i++)
      {
          // diffuse
          vec3 lightDir = normalize(lights[i].Position - fs_in.FragPos);
          float diff = max(dot(lightDir, normal), 0.0);
          vec3 result = lights[i].Color * diff * color;
          // attenuation (use quadratic as we have gamma correction)
          float distance = length(fs_in.FragPos - lights[i].Position);
          result *= 1.0 / (distance * distance);
          lighting += result;

      }
      vec3 result = ambient + lighting;
      FragColor = vec4(result, 1.0);
    }else{
      FragColor = vec4(color, 1.0);
    }

    // check whether FragColor.rgb is higher than some threshold, if so, output as bloom threshold color
    float brightness = dot(FragColor.rgb, vec3(0.2126, 0.7152, 0.0722));
    if(brightness > 1.0)
        BrightColor = vec4(FragColor.rgb, 1.0);
    else
        BrightColor = vec4(0.0, 0.0, 0.0, 1.0);
}