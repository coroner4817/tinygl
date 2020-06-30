#version 410 core
layout (location = 0) in vec3 aPos;
layout (location = 2) in vec2 aTexCoord;
layout (location = 3) in vec3 aNormal;

layout (location = 4) out vec2 TexCoord;
layout (location = 5) out vec3 vertCoord;
layout (location = 6) out vec3 Normal;
layout (location = 7) out vec3 FragPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

// vertex shader will only be call user defined per-vertex
// fragment shader will be call per-fragment and opengl will do the interpolation for all input data to the fragment shader 
// opengl will interpolate all the input data to the fragment shader. 
void main()
{
  gl_Position = projection * view * model * vec4(aPos, 1.0);
  TexCoord = aTexCoord;
  vertCoord = aPos.xyz;
  // need to also update the normal, because we have rotate and we only care about rotation effect to the normal vec
  // we can calc the new norm base on rotation diff
  // Normal = vec3(model * vec4(aNormal, 1.0)) - vec3(model * vec4(0, 0, 0, 1.0));
  // or we can use the normal matrix of the transform mat as follow
  Normal = mat3(transpose(inverse(model))) * aNormal;  
  // convert the fragment pos on on the obj to the world coordinate system
  // so that we can calculate with the lamp world pos
  // we can also do this in the camera space or even the prespective space, as long as we convert all the vector to the same space
  FragPos = vec3(model * vec4(aPos, 1.0));
}