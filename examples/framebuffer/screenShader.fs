#version 410 core
layout (location = 0) out vec4 FragColor;

layout (location = 2) in vec2 TexCoords;

uniform sampler2D screenTexture;

// post-processing
// because we are render the whole scene as a image on the default framebuffer
// we can apply the image filter on the whole scene
void main()
{
  // normal
  // FragColor = vec4(texture(screenTexture, TexCoords).rgb, 1.0);

  // inversion
  // FragColor = vec4(vec3(1.0 - texture(screenTexture, TexCoords)), 1.0);

  // standard grayscale
  // FragColor = texture(screenTexture, TexCoords);
  // float average = (FragColor.r + FragColor.g + FragColor.b) / 3.0;
  // FragColor = vec4(average, average, average, 1.0);

  // green calibrated grayscale
  // FragColor = texture(screenTexture, TexCoords);
  // float average = 0.2126 * FragColor.r + 0.7152 * FragColor.g + 0.0722 * FragColor.b;
  // FragColor = vec4(average, average, average, 1.0);

  // kernel effect
  {
    // offset is the kernel size on the actual image
    const float offset = 1.0 / 300.0;
    vec2 offsets[9] = vec2[](
        vec2(-offset,  offset), // top-left
        vec2( 0.0f,    offset), // top-center
        vec2( offset,  offset), // top-right
        vec2(-offset,  0.0f),   // center-left
        vec2( 0.0f,    0.0f),   // center-center
        vec2( offset,  0.0f),   // center-right
        vec2(-offset, -offset), // bottom-left
        vec2( 0.0f,   -offset), // bottom-center
        vec2( offset, -offset)  // bottom-right
    );
    // coefficients of the effect from the surrounding pixels
    // normal kernel
    // float kernel[9] = float[](
    //     -1, -1, -1,
    //     -1,  9, -1,
    //     -1, -1, -1
    // );

    // blur kernel
    // float kernel[9] = float[](
    //     1.0 / 16, 2.0 / 16, 1.0 / 16,
    //     2.0 / 16, 4.0 / 16, 2.0 / 16,
    //     1.0 / 16, 2.0 / 16, 1.0 / 16
    // );

    // edge detection kernel
    float kernel[9] = float[](
        1, 1, 1,
        1, -8, 1,
        1, 1, 1
    );

    vec3 sampleTex[9];
    for(int i = 0; i < 9; i++)
    {
      // get the pixel val for each pixel in the kernel
      sampleTex[i] = vec3(texture(screenTexture, TexCoords.st + offsets[i]));
    }
    vec3 col = vec3(0.0);
    for(int i = 0; i < 9; i++){
      // accumulate with kernel coefficients
      col += sampleTex[i] * kernel[i];
    }
    FragColor = vec4(col, 1.0);
  }
}