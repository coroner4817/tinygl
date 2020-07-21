#version 410 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in vec3 aTangent;
layout (location = 4) in vec3 aBitangent;

out VS_OUT {
    vec3 FragPos;
    vec2 TexCoords;
    vec3 TangentLightPos;
    vec3 TangentViewPos;
    vec3 TangentFragPos;
} vs_out;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

uniform vec3 lightPos;
uniform vec3 viewPos;

void main()
{
    vs_out.FragPos = vec3(model * vec4(aPos, 1.0));
    vs_out.TexCoords = aTexCoords;

    // the normal readed from the normal map is in the TBN space
    // We cannot use directly use it as the fragment normal
    // So we need to apply the TBN change-basis to the normal to convert that to the world space
    // What we did actually is convert all the attributes to the TBN space and calc the fragment color on the TBN space

    mat3 normalMatrix = transpose(inverse(mat3(model)));
    vec3 T = normalize(normalMatrix * aTangent);
    vec3 N = normalize(normalMatrix * aNormal);
    // re-orthogonalize so that T is 90 degree with N
    // https://zh.wikipedia.org/wiki/%E6%A0%BC%E6%8B%89%E5%A7%86-%E6%96%BD%E5%AF%86%E7%89%B9%E6%AD%A3%E4%BA%A4%E5%8C%96
    T = normalize(T - dot(T, N) * N);
    vec3 B = cross(N, T);

    // TBN is the change-of-basis matrix, so it is different from hand craft rotation matrix
    // rotation matrix is put rotated basis on each row, change-of-basis is put new basis on each row
    // change of basis and rotation matrix is mutual inverse
    // so here we craft a change-basis-matrix to convert the pos from world space to tangent space
    mat3 TBN = transpose(mat3(T, B, N));

    // convert all the position based data to TBN space, so that in the fragment shader we can directly use the value read from the normal map
    vs_out.TangentLightPos = TBN * lightPos;
    vs_out.TangentViewPos  = TBN * viewPos;
    vs_out.TangentFragPos  = TBN * vs_out.FragPos;

    gl_Position = projection * view * model * vec4(aPos, 1.0);
}