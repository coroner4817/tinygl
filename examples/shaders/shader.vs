// Vertex shader:
// ==============
#version 410 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor; // useless here

// out vec3 ourColor;
layout (location = 2) out vec3 ourPosition;

void main()
{
    gl_Position = vec4(aPos, 1.0); 
    ourPosition = aPos;
}