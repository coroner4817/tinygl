#version 410 core
layout (location = 0) out vec4 FragColor;

layout (location = 3) in vec2 TexCoords;

uniform sampler2D texture1;

float near = 0.1;
float far  = 100.0;

float LinearizeDepth(float depth)
{
  // depth is between [0, 1], NDC is between [-1, 1]
  float z = depth * 2.0 - 1.0; // back to NDC
  // base on the formula in the tutorial
  return (2.0 * near * far) / (far + near - z * (far - near));
}

void main()
{
  // FragColor = texture(texture1, TexCoords);
  // FragColor = vec4(vec3(gl_FragCoord.z), 1.0);

  // linear depth which is bad because we need more accurate around near plane
  float depth = LinearizeDepth(gl_FragCoord.z) / far; // divide by far for demonstration
  FragColor = vec4(vec3(depth), 1.0);

  // to prevent z-fighting
  // 1, add offset
  // 2, set the near plane as far as possible
  // 3, use high percision depth buffer
}