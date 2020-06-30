#version 410 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;
layout (location = 2) in vec2 aTexCoord;

layout (location = 3) out vec3 ourColor;
layout (location = 4) out vec2 TexCoord;
layout (location = 5) out vec2 vertCoord;

void main()
{
  gl_Position = vec4(aPos, 1.0);
  ourColor = aColor;
  TexCoord = aTexCoord;
  vertCoord = aPos.xy;
}