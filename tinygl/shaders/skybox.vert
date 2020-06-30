#version 410 core
layout (location = 0) in vec3 aPos;
layout (location = 1) out vec3 FragPos;

uniform mat4 projection;
uniform mat4 view;

void main()
{
    FragPos = aPos;
    vec4 pos = projection * view * vec4(aPos, 1.0);
    // use w instead of z so that after apply the projection matrix
    // the depth of this vertex equal w/w = 1, which is the farest value
    gl_Position = pos.xyww;
}