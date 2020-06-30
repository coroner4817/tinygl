#version 410 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;
layout (location = 3) in vec3 aTangent;
layout (location = 4) in vec3 aBitangent;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

// with interface block we don't need to use layout output
out VS_OUT {
  vec3 normal;
} vs_out;

// vertex shader will only be call user defined per-vertex
// fragment shader will be call per-fragment and opengl will do the interpolation for all input data to the fragment shader
// opengl will interpolate all the input data to the fragment shader.
void main()
{
  gl_Position = projection * view * model * vec4(aPos, 1.0);
  // need to also update the normal, because we have rotate and we only care about rotation effect to the normal vec
  // Normal = vec3(model * vec4(aNormal, 1.0)) - vec3(model * vec4(0, 0, 0, 1.0));
  // or we can use the normal matrix of the transform mat
  vs_out.normal = mat3(transpose(inverse(model))) * aNormal;
}