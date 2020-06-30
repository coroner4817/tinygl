#version 410 core
layout (triangles) in;
layout (triangle_strip, max_vertices=18) out;

uniform mat4 shadowMatrices[6];

out vec4 FragPos; // FragPos from GS (output per emitvertex)

// use
void main()
{
  for(int face = 0; face < 6; ++face)
  {
    gl_Layer = face; // built-in variable that specifies to which face we render.
    // input type is triangle, so that here the gl_in will contain 3 vertices
    // here we bascially convert each vertex to the light space of 6 different faces.
    // because we originally are drawing triangles, we need to keep the same type output but as a stripe
    for(int i = 0; i < 3; ++i) // for each triangle's vertices
    {
      // pass the original world position to the next stage
      FragPos = gl_in[i].gl_Position;

      // augmented to 6 faces so that we can have depth buffer for each faces
      // this generated gl_position is use for next fragment stage to calc the distance, and will trigger next stage 6 times
      // all the vertex need to project to each face. So we need to augmented 6 times

      // basially we have to project all the vertices into each face's light space
      // for each face we will generate a depth buffer with consideration of all vertices.
      // the depth buffer is store in as a texture in the CubeMap typed buffer
      gl_Position = shadowMatrices[face] * FragPos;
      EmitVertex();
    }
    EndPrimitive();
  }
}