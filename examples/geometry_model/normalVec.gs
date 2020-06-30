#version 410 core
layout (triangles) in;
layout (line_strip, max_vertices = 6) out;

// normal vector

in VS_OUT {
  vec3 normal;
} gs_in[];

const float MAGNITUDE = 0.01;

void GenerateLine(int index)
{
  gl_Position = gl_in[index].gl_Position;
  EmitVertex();
  gl_Position = gl_in[index].gl_Position + vec4(gs_in[index].normal, 0.0) * MAGNITUDE;
  EmitVertex();
  EndPrimitive();
}

void main()
{
  // EndPrimitive 3 times so generate 3 lines in the line strip
  GenerateLine(0); // first vertex normal
  GenerateLine(1); // second vertex normal
  GenerateLine(2); // third vertex normal
}