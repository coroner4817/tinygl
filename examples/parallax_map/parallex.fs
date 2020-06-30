#version 410 core
out vec4 FragColor;

in VS_OUT {
    vec3 FragPos;
    vec2 TexCoords;
    vec3 TangentLightPos;
    vec3 TangentViewPos;
    vec3 TangentFragPos;
} fs_in;

uniform sampler2D diffuseMap;
uniform sampler2D normalMap;
uniform sampler2D dispMap;

uniform float heightScale;

// approach 1
// this looks bad on some cases
// vec2 ParallaxMapping(vec2 texCoords, vec3 viewDir)
// {
//     // height map aka dispMap is single value per pixel
//     float height =  texture(dispMap, texCoords).r;
//     // viewDir is normalized, heightScale is the control parameter
//     // so this is just a approximation.
//     return texCoords - viewDir.xy * (height * heightScale);
// }

// approach 2
vec2 ParallaxMapping(vec2 texCoords, vec3 viewDir)
{
    // number of depth layers
    const float minLayers = 8;
    const float maxLayers = 32;
    // becuase in the tangent space so we should use (0,0,1) as the standard normal
    // if the angle between the view dir and the plane normal is small
    // then we can use relative low sample rate
    // because the computation is expensive so we want choose the proper layer number
    // this result in some steep visual issue
    float numLayers = mix(maxLayers, minLayers, abs(dot(vec3(0.0, 0.0, 1.0), viewDir)));
    // calculate the size of each layer
    float layerDepth = 1.0 / numLayers;
    // the amount to shift the texture coordinates per layer (from vector P)
    vec2 P = viewDir.xy / viewDir.z * heightScale;
    vec2 deltaTexCoords = P / numLayers;

    // depth of current layer
    float currentLayerDepth = 0.0;
    // get initial values
    vec2  currentTexCoords     = texCoords;
    float currentdispMapValue = texture(dispMap, currentTexCoords).r;

    // from displacement 0 start sampling until the accumulation layer depth >= currentdispMapValue
    // this is another way to approximate the target coord step by step
    while(currentLayerDepth < currentdispMapValue)
    {
        // shift texture coordinates along direction of P
        currentTexCoords -= deltaTexCoords;
        // get dispMap value at current texture coordinates
        currentdispMapValue = texture(dispMap, currentTexCoords).r;
        // get depth of next layer
        currentLayerDepth += layerDepth;
    }

    // fix the steep visual issue
    // get texture coordinates before collision (reverse 1 operations)
    vec2 prevTexCoords = currentTexCoords + deltaTexCoords;
    // get depth after and before collision for linear interpolation
    float afterDepth  = currentdispMapValue - currentLayerDepth;

    float beforeDepth = texture(dispMap, prevTexCoords).r - currentLayerDepth + layerDepth;

    // interpolation of texture coordinates
    // still approximation but more smooth. So the steep issue is less
    float weight = afterDepth / (afterDepth - beforeDepth);
    vec2 finalTexCoords = prevTexCoords * weight + currentTexCoords * (1.0 - weight);

    return finalTexCoords;
}

void main()
{
    // offset texture coordinates with Parallax Mapping
    vec3 viewDir = normalize(fs_in.TangentViewPos - fs_in.TangentFragPos);
    vec2 texCoords = fs_in.TexCoords;

    // get the updated texCoords base on the height map
    texCoords = ParallaxMapping(fs_in.TexCoords,  viewDir);
    // sanity check
    if(texCoords.x > 1.0 || texCoords.y > 1.0 || texCoords.x < 0.0 || texCoords.y < 0.0)
        discard;

    // obtain normal from normal map
    vec3 normal = texture(normalMap, texCoords).rgb;
    normal = normalize(normal * 2.0 - 1.0);

    // get diffuse color
    vec3 color = texture(diffuseMap, texCoords).rgb;
    // ambient
    vec3 ambient = 0.1 * color;
    // diffuse
    vec3 lightDir = normalize(fs_in.TangentLightPos - fs_in.TangentFragPos);
    float diff = max(dot(lightDir, normal), 0.0);
    vec3 diffuse = diff * color;
    // specular
    vec3 reflectDir = reflect(-lightDir, normal);
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), 32.0);

    vec3 specular = vec3(0.2) * spec;
    FragColor = vec4(ambient + diffuse + specular, 1.0);
}