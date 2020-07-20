#pragma once

#include "tinygl/common.h"
#include "tinygl/texture_manager.h"
#include "tinygl/shader_s.h"

// for skybox must in this order
static const std::vector<std::string> SKYBOX_FACES_NAME = {
  "right.jpg",
  "left.jpg",
  "top.jpg",
  "bottom.jpg",
  "front.jpg",
  "back.jpg"
};

static constexpr float skyboxVertices[] = {
    // positions
    -1.0f,  1.0f, -1.0f,
    -1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,

     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,

    -1.0f,  1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f,  1.0f
};

class SkyBox
{
public:
  SkyBox(const std::string& name, unsigned int activeID, bool isEquirectangular, unsigned int irractiveId, unsigned int prefiltermap, unsigned int brdfLUT){
    activeID_ = activeID;
    irractiveId_ = irractiveId;
    prefiltermapId_ = prefiltermap;
    brdfLUTId_ = brdfLUT;
    isEquirectangular_ = isEquirectangular;
    // cube VAO
    unsigned int VBO;
    glGenVertexArrays(1, &skyboxVAOId_);
    glGenBuffers(1, &VBO);
    glBindVertexArray(skyboxVAOId_);	// note that we bind to a different VAO now
    glBindBuffer(GL_ARRAY_BUFFER, VBO);	// and a different VBO
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0); // because the vertex data is tightly packed we can also specify 0 as the vertex attribute's stride to let OpenGL figure it out
    glEnableVertexAttribArray(0);

    if(!isEquirectangular_){ // normal cube map
      // texture
      stbi_set_flip_vertically_on_load(false);
      glGenTextures(1, &skyboxTextureId_);
      glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTextureId_);

      int width, height, nrChannels;
      for (unsigned int i = 0; i < SKYBOX_FACES_NAME.size(); i++)
      {
        unsigned char *data = stbi_load((ASSETS_SKYBOX_BASEPATH + name + "/" + SKYBOX_FACES_NAME[i]).c_str(), &width, &height, &nrChannels, 0);
        if (data)
        {
          glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                        0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data
          );
          stbi_image_free(data);
        }
        else
        {
          std::cout << "Cubemap texture failed to load at path: " << SKYBOX_FACES_NAME[i] << std::endl;
        }
      }
      glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
      stbi_set_flip_vertically_on_load(true);

      // shader
      cubeMapsShader = std::make_unique<Shader>(COMMON_SHADERS_BASEPATH + "skybox.vert", COMMON_SHADERS_BASEPATH + "skybox.frag");
      // active id is per shader, in this case we only have 1 texture so we don't need to worry about duplicate active id
      cubeMapsShader->setInt("skybox", 0);
    }else{ // Equirectangular map
      // pbr: setup framebuffer
      unsigned int captureFBO;
      unsigned int captureRBO;
      glGenFramebuffers(1, &captureFBO);
      glGenRenderbuffers(1, &captureRBO);

      glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
      glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
      glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 512, 512); // capture at 512 resolution
      glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, captureRBO);

      int width, height, nrChannels;
      float *data = stbi_loadf((ASSETS_SKYBOX_BASEPATH + name + "/" + name + ".hdr").c_str(), &width, &height, &nrChannels, 0);
      unsigned int hdrTexture;
      if (data)
      {
        glGenTextures(1, &hdrTexture);
        glBindTexture(GL_TEXTURE_2D, hdrTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, data); // note how we specify the texture's data value to be float

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        stbi_image_free(data);
      }
      else
      {
        std::cout << "Failed to load HDR image." << std::endl;
      }

      // setup cubemap buffer
      glGenTextures(1, &skyboxTextureId_);
      glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTextureId_);
      for (unsigned int i = 0; i < 6; ++i)
      {
          glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 512, 512, 0, GL_RGB, GL_FLOAT, nullptr);
      }
      glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

      // set up projection and view matrice
      glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
      glm::mat4 captureViews[] =
      {
          glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
          glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
          glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
          glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
          glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
          glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
      };

      Shader equirShader(COMMON_SHADERS_BASEPATH + "posBypass.vs", COMMON_SHADERS_BASEPATH + "equirectangular.fs");
      equirShader.use();
      equirShader.setInt("equirectangularMap", 0);
      equirShader.setMatrix4fv("projection", captureProjection);
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, hdrTexture);

      // map the equirectangular map to the cubemap
      glViewport(0, 0, 512, 512); // don't forget to configure the viewport to the capture dimensions.
      glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
      for (unsigned int i = 0; i < 6; ++i)
      {
        equirShader.setMatrix4fv("view", captureViews[i]);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, skyboxTextureId_, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // render Cube
        glBindVertexArray(skyboxVAOId_);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
      }
      glBindFramebuffer(GL_FRAMEBUFFER, 0);

      // then let OpenGL generate mipmaps from first mip face (combatting visible dots artifact)
      glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTextureId_);
      glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

      // convolute the diffuse irradianceMap, 32x32 is enough
      // another option is to use the env material file included in the resource
      // pbr: create an irradiance cubemap, and re-scale capture FBO to irradiance scale.
      glGenTextures(1, &skyboxIrradianceId_);
      glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxIrradianceId_);
      for (unsigned int i = 0; i < 6; ++i)
      {
          glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 32, 32, 0, GL_RGB, GL_FLOAT, nullptr);
      }
      glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

      glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
      glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
      glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 32, 32);

      Shader irradianceShader(COMMON_SHADERS_BASEPATH + "posBypass.vs", COMMON_SHADERS_BASEPATH + "irradiance.fs");
      irradianceShader.use();
      irradianceShader.setInt("environmentMap", 0);
      irradianceShader.setMatrix4fv("projection", captureProjection);
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTextureId_);

      glViewport(0, 0, 32, 32); // don't forget to configure the viewport to the capture dimensions.
      glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
      for (unsigned int i = 0; i < 6; ++i)
      {
        irradianceShader.setMatrix4fv("view", captureViews[i]);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, skyboxIrradianceId_, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // render Cube
        glBindVertexArray(skyboxVAOId_);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
      }
      glBindFramebuffer(GL_FRAMEBUFFER, 0);

      // for specular
      // pbr: create a pre-filter cubemap, and re-scale capture FBO to pre-filter scale.
      // --------------------------------------------------------------------------------
      glGenTextures(1, &prefilterMap);
      glBindTexture(GL_TEXTURE_CUBE_MAP, prefilterMap);
      for (unsigned int i = 0; i < 6; ++i)
      {
          glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 128, 128, 0, GL_RGB, GL_FLOAT, nullptr);
      }
      glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); // be sure to set minifcation filter to mip_linear
      glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      // generate mipmaps for the cubemap so OpenGL automatically allocates the required memory.
      glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

      // pbr: run a quasi monte-carlo simulation on the environment lighting to create a prefilter (cube)map.
      // ----------------------------------------------------------------------------------------------------
      Shader prefilterShader(COMMON_SHADERS_BASEPATH + "posBypass.vs", COMMON_SHADERS_BASEPATH + "prefilter.fs");
      prefilterShader.use();
      prefilterShader.setInt("environmentMap", 0);
      prefilterShader.setMatrix4fv("projection", captureProjection);
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTextureId_);

      glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
      unsigned int maxMipLevels = 5;
      for (unsigned int mip = 0; mip < maxMipLevels; ++mip)
      {
          // reisze framebuffer according to mip-level size.
          unsigned int mipWidth  = 128 * std::pow(0.5, mip);
          unsigned int mipHeight = 128 * std::pow(0.5, mip);
          glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
          glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, mipWidth, mipHeight);
          glViewport(0, 0, mipWidth, mipHeight);

          float roughness = (float)mip / (float)(maxMipLevels - 1);
          prefilterShader.setFloat("roughness", roughness);
          for (unsigned int i = 0; i < 6; ++i)
          {
              prefilterShader.setMatrix4fv("view", captureViews[i]);
              glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, prefilterMap, mip);
              glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

              // render cube
              glBindVertexArray(skyboxVAOId_);
              glDrawArrays(GL_TRIANGLES, 0, 36);
              glBindVertexArray(0);
          }
      }
      glBindFramebuffer(GL_FRAMEBUFFER, 0);

      // pbr: generate a 2D LUT from the BRDF equations used.
      // ----------------------------------------------------
      glGenTextures(1, &brdfLUTTexture);

      // pre-allocate enough memory for the LUT texture.
      glBindTexture(GL_TEXTURE_2D, brdfLUTTexture);
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, 512, 512, 0, GL_RG, GL_FLOAT, 0);
      // be sure to set wrapping mode to GL_CLAMP_TO_EDGE
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

      // then re-configure capture framebuffer object and render screen-space quad with BRDF shader.
      glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
      glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
      glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 512, 512);
      glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, brdfLUTTexture, 0);

      glViewport(0, 0, 512, 512);
      Shader brdfShader(COMMON_SHADERS_BASEPATH + "framePass.vs", COMMON_SHADERS_BASEPATH + "brdf.fs");
      brdfShader.use();
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      {// render quad
        unsigned int quadVAO, quadVBO;
        float quadVertices[] = {
            // positions        // texture Coords
            -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
            -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
             1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
             1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
        };
        // setup plane VAO
        glGenVertexArrays(1, &quadVAO);
        glGenBuffers(1, &quadVBO);
        glBindVertexArray(quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));

        glBindVertexArray(quadVAO);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        glBindVertexArray(0);
      }
      glBindFramebuffer(GL_FRAMEBUFFER, 0);

      // create the shader that will render the actual mesh and texture
      cubeMapsShader = std::make_unique<Shader>(COMMON_SHADERS_BASEPATH + "skybox.vert", COMMON_SHADERS_BASEPATH + "environment.fs");
      cubeMapsShader->setInt("environmentMap", activeID_);
    }
  }
  ~SkyBox(){
    // comment out because crash on Ubuntu 16.04
    // glDeleteVertexArrays(1, &skyboxVAOId_);
    // glDeleteTextures(6, &skyboxTextureId_);
  };

  void use(const glm::mat4& view, const glm::mat4& proj){
    glDepthFunc(GL_LEQUAL);
    cubeMapsShader->use();
    cubeMapsShader->setMatrix4fv("view", view);
    cubeMapsShader->setMatrix4fv("projection", proj);

    glBindVertexArray(skyboxVAOId_);
    glActiveTexture(GL_TEXTURE0 + activeID_);
    glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTextureId_);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glDepthFunc(GL_LESS);
    glBindVertexArray(0);
  };

  void useCubemapTexture(){
    glActiveTexture(GL_TEXTURE0 + activeID_);
    glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTextureId_);
  }

  void useIBLTexture(){
    if(isEquirectangular_){
      glActiveTexture(GL_TEXTURE0 + irractiveId_);
      glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxIrradianceId_);
      glActiveTexture(GL_TEXTURE0 + prefiltermapId_);
      glBindTexture(GL_TEXTURE_CUBE_MAP, prefilterMap);
      glActiveTexture(GL_TEXTURE0 + brdfLUTId_);
      glBindTexture(GL_TEXTURE_2D, brdfLUTTexture);
    }
  }

private:
  bool isEquirectangular_;
  unsigned int skyboxTextureId_;
  unsigned int skyboxIrradianceId_;
  unsigned int skyboxVAOId_;
  unsigned int activeID_;
  unsigned int irractiveId_;
  unsigned int prefiltermapId_;
  unsigned int prefilterMap;
  unsigned int brdfLUTId_;
  unsigned int brdfLUTTexture;
  std::unique_ptr<Shader> cubeMapsShader = nullptr;
};