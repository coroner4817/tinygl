#version 410 core
// output to different color buffer belong to the same framebuffer
// output sequence is according to the attachment sequence
layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec3 gAlbedo;

layout (location = 3) in vec3 FragPos;
layout (location = 4) in vec2 TexCoords;
layout (location = 5) in vec3 Normal;


void main()
{
    // store the fragment position vector in the first gbuffer texture
    gPosition = FragPos;
    // also store the per-fragment normals into the gbuffer
    gNormal = normalize(Normal);
    // and the diffuse per-fragment color
    gAlbedo.rgb = vec3(0.95);
}