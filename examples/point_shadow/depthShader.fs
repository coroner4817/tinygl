#version 410 core
in vec4 FragPos;

uniform vec3 lightPos;
uniform float far_plane;

void main()
{
    // calc the distance in the world space
    // this is same for all the 6 faces
    float lightDistance = length(FragPos.xyz - lightPos);

    // map to [0, 1] range by dividing by far_plane
    lightDistance = lightDistance / far_plane;

    // write this as modified depth
    gl_FragDepth = lightDistance;
}