#version 410 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

layout (location = 3) out vec3 FragPos;
layout (location = 4) out vec2 TexCoords;
layout (location = 5) out vec3 Normal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform bool invertedNormals;

void main()
{
    vec4 worldPos = model * vec4(aPos, 1.0);
    FragPos = worldPos.xyz;
    TexCoords = aTexCoords;

    mat3 normalMatrix = transpose(inverse(mat3(model)));
    Normal = normalMatrix * (invertedNormals ? -aNormal : aNormal);

    gl_Position = projection * view * worldPos;
}