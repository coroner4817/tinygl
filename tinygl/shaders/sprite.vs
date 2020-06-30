#version 410 core
// compact the buffer into a single stream
layout (location = 0) in vec4 vertex; // <vec2 pos, vec2 tex>
layout (location = 1) out vec2 TexCoords;

uniform mat4 projection;
uniform mat4 model;

void main()
{
    gl_Position = projection * model * vec4(vertex.xy, 0.0, 1.0);
    TexCoords = vertex.zw;
}
