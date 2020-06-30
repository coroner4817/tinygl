#version 410 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;
layout (location = 3) in vec3 aTangent;
layout (location = 4) in vec3 aBitangent;

// layout (location = 5) out vec2 TexCoord;
// layout (location = 6) out vec3 Normal;
// layout (location = 7) out vec3 FragPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

// with interface block we don't need to use layout output
out VS_OUT {
  vec2 texCoords;
  vec3 normal;
  vec3 fragPos;
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
  // convert the fragment pos on on the obj to the world coordinate system
  // so that we can calculate with the lamp world pos
  // we can also do this in the camera space or even the prespective space, as long as we convert all the vector to the same space
  vs_out.fragPos = vec3(model * vec4(aPos, 1.0));

  vs_out.texCoords = aTexCoord;
}