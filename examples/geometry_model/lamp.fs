#version 410 core
layout (location = 0) out vec4 FragColor;
layout (location = 1) in vec3 FragPos;

void main()
{
  FragColor = vec4(clamp(normalize(FragPos), vec3(0.3), vec3(1)), 1.0); // set all 4 vector values to 1.0
}