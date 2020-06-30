#version 410 core
// in is points means there is only 1 vertex pre primitive input
layout (points) in;
// out is primitive type is triangle_strip, so when call EndPrimitive,
// it will combine in the triangle strip fashion
// max_vertices can be any number, just set the max output number so that GPU can alocate memory
layout (triangle_strip, max_vertices = 5) out;

// input from the vertex shader
in VS_OUT {
    vec3 color;
} gs_in[];

// out to the fragment shader
out vec3 fColor;

void build_house(vec4 position)
{
  fColor = gs_in[0].color; // gs_in[0] since there's only one input vertex
  gl_Position = position + vec4(-0.2, -0.2, 0.0, 0.0);    // 1:bottom-left
  EmitVertex();
  gl_Position = position + vec4( 0.2, -0.2, 0.0, 0.0);    // 2:bottom-right
  EmitVertex();
  gl_Position = position + vec4(-0.2,  0.2, 0.0, 0.0);    // 3:top-left
  EmitVertex();
  gl_Position = position + vec4( 0.2,  0.2, 0.0, 0.0);    // 4:top-right
  EmitVertex();
  gl_Position = position + vec4( 0.0,  0.4, 0.0, 0.0);    // 5:top
  // note that this fColor will be fire to the last vertex!
  fColor = vec3(1.0, 1.0, 1.0);
  EmitVertex();

  // when EndPrimitive get called, all these emit above will be combined
  // into a primitive
  EndPrimitive();
}

void main()
{
  // we only have 1 vertext input here
  build_house(gl_in[0].gl_Position);
}
