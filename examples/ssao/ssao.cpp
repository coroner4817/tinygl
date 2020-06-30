#include "tinygl/glApp.h"
#include "tinygl/camera.h"
#include "tinygl/shader_s.h"
#include "tinygl/model_loader.h"
#include "tinygl/default_mesh.h"
#include <random>

// attenuation parameters
const float constant = 1.0; // note that we don't send this to the shader, we assume it is always 1.0 (in our case)
const float linear = 0.09;
const float quadratic = 0.032;

// light info
glm::vec3 lightPos = glm::vec3(2.0, 4.0, -2.0);
glm::vec3 lightColor = glm::vec3(0.2, 0.2, 0.7);

// user define vertex structure, because only user know the input model vertex data format
struct Vertex
{
  glm::vec3 Position;
  glm::vec3 Normal;
  glm::vec2 TexCoords;
  glm::vec3 Tangent;
  glm::vec3 Bitangent;
};

// user defined load texture type
std::vector<aiTextureType> nanosuitTextureTypes = {aiTextureType_DIFFUSE, aiTextureType_SPECULAR};

float lerp(float a, float b, float f)
{
  return a + f * (b - a);
}

// TODO: need to fully understand SSAO
// SSAO reduce the ambient light factor when there are some obj around the render target
int main()
{
  GLApp app;
  app.init();
  app.setProjectionMatrix(45.f, 0.1f, 100.f);
  Camera cam;
  cam.addViewMatrixUpdateListener(&app);
  DefaultMesh dm(&app, &cam);
  dm.initCubeData();
  dm.initQuadData();

  app.generateNewVAOFromArray("cube", dm.getCubeVerticesArray(), dm.getCubeVerticesArraySize(), {{0, 3}, {1, 2}, {2, 3}}, dm.getCubeIndicesArray(), dm.getCubeIndicesArraySize());
  app.generateNewVAOFromArray("quad", dm.getQuadVerticesArray(), dm.getQuadVerticesArraySize(), {{0, 3}, {1, 2}}, dm.getQuadIndicesArray(), dm.getQuadIndicesArraySize());

  auto modelLoader = ModelLoader<Vertex>(3);
  modelLoader.setProcessMeshHandler(STANDARD_OBJ_LOADER(Vertex, nanosuitTextureTypes));
  modelLoader.load(ASSETS_3D_BASEPATH + "nanosuit/nanosuit.obj", "nanosuit");

  Shader gbufferShader(getExecutablePath() + "/gbufferShader.vs", getExecutablePath() + "/gbufferShader.fs");
  gbufferShader.init("model", "view", &cam, "projection", app.getProjectionMatrix());

  Shader ssaoShader(COMMON_SHADERS_BASEPATH + "/framePass.vs", getExecutablePath() + "/ssaoShader.fs");
  ssaoShader.use();
  ssaoShader.setInt("gPosition", 0);
  ssaoShader.setInt("gNormal", 1);
  ssaoShader.setInt("texNoise", 2);

  Shader blurShader(COMMON_SHADERS_BASEPATH + "/framePass.vs", getExecutablePath() + "/blurShader.fs");
  blurShader.use();
  blurShader.setInt("ssaoInput", 0);

  Shader lightShader(COMMON_SHADERS_BASEPATH + "/framePass.vs", getExecutablePath() + "/lightShader.fs");
  lightShader.use();
  lightShader.setInt("gPosition", 0);
  lightShader.setInt("gNormal", 1);
  lightShader.setInt("gAlbedo", 2);
  lightShader.setInt("ssao", 3);
  lightShader.setVec3("light.Color", lightColor);
  lightShader.setFloat("light.Linear", linear);
  lightShader.setFloat("light.Quadratic", quadratic);

  // configure g-buffer framebuffer
  // ------------------------------
  unsigned int gBuffer;
  glGenFramebuffers(1, &gBuffer);
  glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
  unsigned int gPosition, gNormal, gAlbedo;
  // position color buffer
  glGenTextures(1, &gPosition);
  glBindTexture(GL_TEXTURE_2D, gPosition);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, app.SCR_WIDTH, app.SCR_HEIGHT, 0, GL_RGB, GL_FLOAT, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gPosition, 0);
  // normal color buffer
  glGenTextures(1, &gNormal);
  glBindTexture(GL_TEXTURE_2D, gNormal);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, app.SCR_WIDTH, app.SCR_HEIGHT, 0, GL_RGB, GL_FLOAT, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gNormal, 0);
  // color + specular color buffer
  glGenTextures(1, &gAlbedo);
  glBindTexture(GL_TEXTURE_2D, gAlbedo);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, app.SCR_WIDTH, app.SCR_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gAlbedo, 0);
  // tell OpenGL which color attachments we'll use (of this framebuffer) for rendering
  unsigned int attachments[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
  glDrawBuffers(3, attachments);
  // create and attach depth buffer (renderbuffer)
  unsigned int rboDepth;
  glGenRenderbuffers(1, &rboDepth);
  glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, app.SCR_WIDTH, app.SCR_HEIGHT);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);
  // finally check if framebuffer is complete
  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
      std::cout << "Framebuffer not complete!" << std::endl;
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  // also create framebuffer to hold SSAO processing stage
  // -----------------------------------------------------
  unsigned int ssaoFBO, ssaoBlurFBO;
  glGenFramebuffers(1, &ssaoFBO);  glGenFramebuffers(1, &ssaoBlurFBO);
  glBindFramebuffer(GL_FRAMEBUFFER, ssaoFBO);
  unsigned int ssaoColorBuffer, ssaoColorBufferBlur;
  // SSAO color buffer
  glGenTextures(1, &ssaoColorBuffer);
  glBindTexture(GL_TEXTURE_2D, ssaoColorBuffer);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, app.SCR_WIDTH, app.SCR_HEIGHT, 0, GL_RGB, GL_FLOAT, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ssaoColorBuffer, 0);
  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
      std::cout << "SSAO Framebuffer not complete!" << std::endl;
  // and blur stage
  glBindFramebuffer(GL_FRAMEBUFFER, ssaoBlurFBO);
  glGenTextures(1, &ssaoColorBufferBlur);
  glBindTexture(GL_TEXTURE_2D, ssaoColorBufferBlur);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, app.SCR_WIDTH, app.SCR_HEIGHT, 0, GL_RGB, GL_FLOAT, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ssaoColorBufferBlur, 0);
  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
      std::cout << "SSAO Blur Framebuffer not complete!" << std::endl;
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  // generate sample kernel
  // ----------------------
  std::uniform_real_distribution<GLfloat> randomFloats(0.0, 1.0); // generates random floats between 0.0 and 1.0
  std::default_random_engine generator;
  std::vector<glm::vec3> ssaoKernel;
  for (unsigned int i = 0; i < 64; ++i)
  {
      glm::vec3 sample(randomFloats(generator) * 2.0 - 1.0, randomFloats(generator) * 2.0 - 1.0, randomFloats(generator));
      sample = glm::normalize(sample);
      sample *= randomFloats(generator);
      float scale = float(i) / 64.0;

      // scale samples s.t. they're more aligned to center of kernel
      scale = lerp(0.1f, 1.0f, scale * scale);
      sample *= scale;
      ssaoKernel.push_back(sample);
  }

  // generate noise texture
  // ----------------------
  std::vector<glm::vec3> ssaoNoise;
  for (unsigned int i = 0; i < 16; i++)
  {
      glm::vec3 noise(randomFloats(generator) * 2.0 - 1.0, randomFloats(generator) * 2.0 - 1.0, 0.0f); // rotate around z-axis (in tangent space)
      ssaoNoise.push_back(noise);
  }
  unsigned int noiseTexture; glGenTextures(1, &noiseTexture);
  glBindTexture(GL_TEXTURE_2D, noiseTexture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, 4, 4, 0, GL_RGB, GL_FLOAT, &ssaoNoise[0]);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

  ssaoShader.use();
  // Send kernel + rotation
  for (unsigned int i = 0; i < 64; ++i)
      ssaoShader.setVec3("samples[" + std::to_string(i) + "]", ssaoKernel[i]);
  ssaoShader.setMatrix4fv("projection", app.getProjectionMatrix());

  app.run([=, &gbufferShader, &lightShader, &ssaoShader, &blurShader, &modelLoader, &cam, &app, &dm](float a_delta){
    cam.update(app.getWindow(), a_delta);

    // 1. geometry pass: render scene's geometry/color data into gbuffer
    // -----------------------------------------------------------------
    gbufferShader.use();{
      glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        // room cube
        app.useVAO("cube");
        gbufferShader.setModelMatrix({0.0, 7.0f, 0.0}, glm::vec3(15));
        gbufferShader.setInt("invertedNormals", true); // flip normal because we are inside
        glDrawElements(GL_TRIANGLES, dm.getCubeVerticesCount(), GL_UNSIGNED_INT, 0);
        gbufferShader.setInt("invertedNormals", false);
        // model on floor
        gbufferShader.setModelMatrix({0.0f, 0.0f, 5.0f}, glm::vec3(.5f, .5f, .5f), glm::radians(-90.f), {1.0, 0.0, 0.0});
        modelLoader.Draw({"nanosuit"}, gbufferShader);
      glBindFramebuffer(GL_FRAMEBUFFER, 0);
    };

    // 2. generate SSAO texture, dump to the color attachment of ssaoFBO
    // ------------------------
    ssaoShader.use();{
      glBindFramebuffer(GL_FRAMEBUFFER, ssaoFBO);
        glClear(GL_COLOR_BUFFER_BIT);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, gPosition);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, gNormal);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, noiseTexture);
        app.useVAO("quad");
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
      glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    // 3. blur SSAO texture to remove noise
    // ------------------------------------
    blurShader.use();{
      glBindFramebuffer(GL_FRAMEBUFFER, ssaoBlurFBO);
        glClear(GL_COLOR_BUFFER_BIT);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, ssaoColorBuffer); // bind output from last stage
        app.useVAO("quad");
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
      glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    // 4. lighting pass: traditional deferred Blinn-Phong lighting with added screen-space ambient occlusion
    // -----------------------------------------------------------------------------------------------------
    lightShader.use();{
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      glm::vec3 lightPosView = glm::vec3(cam.GetViewMatrix() * glm::vec4(lightPos, 1.0));
      lightShader.setVec3("light.Position", lightPosView);
      lightShader.setVec3("viewPos", cam.GetCameraPos());
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, gPosition);
      glActiveTexture(GL_TEXTURE1);
      glBindTexture(GL_TEXTURE_2D, gNormal);
      glActiveTexture(GL_TEXTURE2);
      glBindTexture(GL_TEXTURE_2D, gAlbedo);
      // glActiveTexture(GL_TEXTURE3); // add extra SSAO texture to lighting pass
      // glBindTexture(GL_TEXTURE_2D, ssaoColorBufferBlur);
      app.useVAO("quad");
      glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    }
  });
}