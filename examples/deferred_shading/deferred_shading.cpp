#include "tinygl/glApp.h"
#include "tinygl/camera.h"
#include "tinygl/shader_s.h"
#include "tinygl/model_loader.h"
#include "tinygl/default_mesh.h"

std::vector<glm::vec3> objectPositions = {
  glm::vec3(-3.0,  -3.0, -3.0),
  glm::vec3( 0.0,  -3.0, -3.0),
  glm::vec3( 3.0,  -3.0, -3.0),
  glm::vec3(-3.0,  -3.0,  0.0),
  glm::vec3( 0.0,  -3.0,  0.0),
  glm::vec3( 3.0,  -3.0,  0.0),
  glm::vec3(-3.0,  -3.0,  3.0),
  glm::vec3( 0.0,  -3.0,  3.0),
  glm::vec3( 3.0,  -3.0,  3.0),
};

// attenuation parameters
const float constant = 1.0; // note that we don't send this to the shader, we assume it is always 1.0 (in our case)
const float linear = 0.7;
const float quadratic = 1.8;

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

// the reason that we need to use deferred buffer is if we render this scene normally there will be too many calculation
// we have to calc light for each texel. While if using deferred buffer, we only need to calc light for the texel of the window size
int main()
{
  GLApp app;
  app.init();
  app.setProjectionMatrix(45.f, 0.1f, 100.f);
  Camera cam;
  cam.addViewMatrixUpdateListener(&app);
  DefaultMesh dm(&app, &cam);
  dm.initCube();
  dm.initQuadData();

  app.generateNewVAOFromArray("quad", dm.getQuadVerticesArray(), dm.getQuadVerticesArraySize(), {{0, 3}, {1, 2}}, dm.getQuadIndicesArray(), dm.getQuadIndicesArraySize());

  auto modelLoader = ModelLoader<Vertex>(3);
  modelLoader.setProcessMeshHandler(STANDARD_OBJ_LOADER(Vertex, nanosuitTextureTypes));
  modelLoader.load(ASSETS_3D_BASEPATH + "nanosuit/nanosuit.obj", "nanosuit");

  Shader gbufferShader(getExecutablePath() + "/gbufferShader.vs", getExecutablePath() + "/gbufferShader.fs");
  gbufferShader.init("model", "view", &cam, "projection", app.getProjectionMatrix());
  gbufferShader.setTextureBaseName(aiTextureType_DIFFUSE, "texture_diffuse");
  gbufferShader.setTextureBaseName(aiTextureType_SPECULAR, "texture_specular");

  Shader deferredShader(getExecutablePath() + "/deferredShader.vs", getExecutablePath() + "/deferredShader.fs");
  deferredShader.use();
  deferredShader.setInt("gPosition", 0);
  deferredShader.setInt("gNormal", 1);
  deferredShader.setInt("gAlbedoSpec", 2);

  // random 32 lighting info
  // -------------
  const unsigned int NR_LIGHTS = 32;
  std::vector<glm::vec3> lightPositions;
  std::vector<glm::vec3> lightColors;
  srand(13);
  for (unsigned int i = 0; i < NR_LIGHTS; i++)
  {
      // calculate slightly random offsets
      float xPos = ((rand() % 100) / 100.0) * 6.0 - 3.0;
      float yPos = ((rand() % 100) / 100.0) * 6.0 - 4.0;
      float zPos = ((rand() % 100) / 100.0) * 6.0 - 3.0;
      lightPositions.push_back(glm::vec3(xPos, yPos, zPos));
      // also calculate random color
      float rColor = ((rand() % 100) / 200.0f) + 0.5; // between 0.5 and 1.0
      float gColor = ((rand() % 100) / 200.0f) + 0.5; // between 0.5 and 1.0
      float bColor = ((rand() % 100) / 200.0f) + 0.5; // between 0.5 and 1.0
      lightColors.push_back(glm::vec3(rColor, gColor, bColor));
  }

  // configure g-buffer framebuffer
  // ------------------------------
  unsigned int gBuffer;
  glGenFramebuffers(1, &gBuffer);
  glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
  unsigned int gPosition, gNormal, gAlbedoSpec;
  // position color buffer
  glGenTextures(1, &gPosition);
  glBindTexture(GL_TEXTURE_2D, gPosition);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, app.SCR_WIDTH, app.SCR_HEIGHT, 0, GL_RGB, GL_FLOAT, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gPosition, 0);
  // normal color buffer
  glGenTextures(1, &gNormal);
  glBindTexture(GL_TEXTURE_2D, gNormal);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, app.SCR_WIDTH, app.SCR_HEIGHT, 0, GL_RGB, GL_FLOAT, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gNormal, 0);
  // color + specular color buffer
  glGenTextures(1, &gAlbedoSpec);
  glBindTexture(GL_TEXTURE_2D, gAlbedoSpec);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, app.SCR_WIDTH, app.SCR_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gAlbedoSpec, 0);
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

  app.run([=, &gbufferShader, &deferredShader, &modelLoader, &cam, &app, &dm](float a_delta){
    cam.update(app.getWindow(), a_delta);

    // 1, first pass, also called forward rendering
    gbufferShader.use();{
      glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        for (unsigned int i = 0; i < objectPositions.size(); i++)
        {
          gbufferShader.setModelMatrix(objectPositions[i], glm::vec3(.2f, .2f, .2f));
          modelLoader.Draw({"nanosuit"}, gbufferShader);
        }
      glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    // 2, calc light effects and merge, also called deferred rendering
    deferredShader.use();{
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, gPosition);
      glActiveTexture(GL_TEXTURE1);
      glBindTexture(GL_TEXTURE_2D, gNormal);
      glActiveTexture(GL_TEXTURE2);
      glBindTexture(GL_TEXTURE_2D, gAlbedoSpec);
      deferredShader.setVec3("viewPos", cam.GetCameraPos());
      // send light relevant uniforms
      for (unsigned int i = 0; i < lightPositions.size(); i++)
      {
        deferredShader.setVec3("lights[" + std::to_string(i) + "].Position", lightPositions[i]);
        deferredShader.setVec3("lights[" + std::to_string(i) + "].Color", lightColors[i]);
        deferredShader.setFloat("lights[" + std::to_string(i) + "].Linear", linear);
        deferredShader.setFloat("lights[" + std::to_string(i) + "].Quadratic", quadratic);
        // then calculate radius of light volume/sphere
        const float maxBrightness = std::fmaxf(std::fmaxf(lightColors[i].r, lightColors[i].g), lightColors[i].b);
        // explaining of the equaltion check the turtorial
        // And also check the "How we really use light volumes" section!!!!!
        float radius = (-linear + std::sqrt(linear * linear - 4 * quadratic * (constant - (256.0f / 5.0f) * maxBrightness))) / (2.0f * quadratic);
        deferredShader.setFloat("lights[" + std::to_string(i) + "].Radius", radius);
      }
      app.useVAO("quad");
      glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    }

    // 2.5. copy content of geometry's depth buffer to default framebuffer's depth buffer
    // Just need to copy the depth buffer because the colorbuffer are already rendered on the quad
    // This is because we will still rendering the lamps afterward
    // if we don't pass the proper depth buffer to the default framebuffer
    // the depth from the last render will be the closet so that lamps will not show up
    // ----------------------------------------------------------------------------------
    glBindFramebuffer(GL_READ_FRAMEBUFFER, gBuffer);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0); // write to default framebuffer
    // blit to default framebuffer. Note that this may or may not work as the internal formats of both the FBO and default framebuffer have to match.
    // the internal formats are implementation defined. This works on all of my systems, but if it doesn't on yours you'll likely have to write to the
    // depth buffer in another shader stage (or somehow see to match the default framebuffer's internal format with the FBO's internal format).
    glBlitFramebuffer(0, 0, app.SCR_WIDTH, app.SCR_HEIGHT, 0, 0, app.SCR_WIDTH, app.SCR_HEIGHT, GL_DEPTH_BUFFER_BIT, GL_NEAREST); // copy the depth buffer from the gbuffer to the original
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // 3, render lights
    for(int i = 0; i < lightPositions.size(); ++i){
      dm.drawCube(lightColors[i], lightPositions[i], glm::vec3(0.25));
    }
  });
}