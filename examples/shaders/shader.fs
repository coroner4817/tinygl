// Fragment shader:
// ================
#version 410 core
layout (location = 0) out vec4 FragColor;
layout (location = 2) in vec3 ourPosition;

void main()
{
    FragColor = vec4(ourPosition, 1.0);    // note how the position value is linearly interpolated to get all the different colors
}