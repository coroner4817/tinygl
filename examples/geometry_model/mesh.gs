#version 410 core
layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

// explosion

in VS_OUT {
  vec2 texCoords;
  vec3 normal;
  vec3 fragPos;
} gs_in[];

// pass to the fragment shader
out vec2 TexCoord;
out vec3 Normal;
out vec3 FragPos;

uniform float explode_time;

vec4 explode(vec4 position, vec3 normal)
{
  float magnitude = 0.025;
  vec3 direction = normal * ((sin(explode_time) + 1.0) / 2.0) * magnitude;
  return position + vec4(direction, 0.0);
}

void main() {
  // for each vertex, forward the input data for each vertex
  TexCoord = gs_in[0].texCoords;
  Normal = gs_in[0].normal;
  FragPos = gs_in[0].fragPos;
  gl_Position = explode(gl_in[0].gl_Position, Normal);
  EmitVertex();

  TexCoord = gs_in[1].texCoords;
  Normal = gs_in[1].normal;
  FragPos = gs_in[1].fragPos;
  gl_Position = explode(gl_in[1].gl_Position, Normal);
  EmitVertex();

  TexCoord = gs_in[2].texCoords;
  Normal = gs_in[2].normal;
  FragPos = gs_in[2].fragPos;
  gl_Position = explode(gl_in[2].gl_Position, Normal);
  EmitVertex();

  EndPrimitive();
}