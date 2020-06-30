#version 410 core
layout (location = 0) out vec4 color;
layout (location = 1) in vec2 TexCoords;

uniform sampler2D text; // texture ID of the character
uniform vec3 textColor;

void main()
{
  // texture(text, TexCoords).r is either 0 or 1
  // so when dot textcolor, if is 0 then will not render (result alpha=0)
  // if 1 then render the color
  // this can save us a if check
  vec4 sampled = vec4(1.0, 1.0, 1.0, texture(text, TexCoords).r);
  color = vec4(textColor, 1.0) * sampled;

  // this not require to blend
  // color = vec4(textColor, 1.0) * vec4(texture(text, TexCoords).r, texture(text, TexCoords).r, texture(text, TexCoords).r, 1.0);
}