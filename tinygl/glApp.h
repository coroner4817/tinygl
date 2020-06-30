#pragma once

#include "tinygl/common.h"
#include "tinygl/camera.h"
#include "tinygl/texture_manager.h"
#include "tinygl/VAO_manager.h"
#include "tinygl/skybox.h"
#include "tinygl/UBO_manager.h"

void framebuffer_size_callback(GLFWwindow* window, int width, int height);

static void error_callback(int error, const char* description)
{
  fputs(description, stderr);
}

class GLApp : public CameraViewMatrixUpdateListener,
              public TextureManager,
              public VAOManager,
              public UBOManager
{
public:
  GLApp()
  {
  }
  GLApp(int w, int h, glm::vec4 cc={0.1, 0.1, 0.1, 1.0})
  {
    SCR_WIDTH = w;
    SCR_HEIGHT = h;
    clearColor = cc;
  }
  ~GLApp()
  {
    terminate();
  }

  GLuint getScreenWidth() const { return SCR_WIDTH; }
  GLuint getScreenHeight() const { return SCR_HEIGHT; }

  unsigned int SCR_WIDTH = 1280;
  unsigned int SCR_HEIGHT = 720;
  glm::vec4 clearColor = {0.1, 0.1, 0.1, 1.0};

  void init()
  {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // anti-alias MSAA
    // average the fragment at the edge, make the edge little blured
    // glfwWindowHint(GLFW_SAMPLES, 4);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // uncomment this statement to fix compilation on OS X
#endif

    // glfw window creation
    // --------------------
    window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
    if (window == NULL)
    {
      std::cout << "Failed to create GLFW window" << std::endl;
      glfwTerminate();
      return;
    }
    glfwMakeContextCurrent(window);
    glfwSetErrorCallback(error_callback);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
      std::cout << "Failed to initialize GLAD" << std::endl;
      return;
    }
    stbi_set_flip_vertically_on_load(true);

    // enable MSAA anti-alias
    // glEnable(GL_MULTISAMPLE);
    // enable depth test when drawing
    glEnable(GL_DEPTH_TEST);
    // LESS is the default compare func
    glDepthFunc(GL_LESS);

    // use face culling to improve preformance
    // only render the Counter-clock wise front faces that is facing to the viewport
    // noted that this required to have the vertices data in the CCW format
    // glEnable(GL_CULL_FACE);
    // glCullFace(GL_FRONT);
    // glFrontFace(GL_CCW);

    // wire frame mode
    // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    // enable sRGB (gamma 2.2) correction
    // convert sRGB to linear color space
    // glEnable(GL_FRAMEBUFFER_SRGB);
  }

  void enableMouseCallback()
  {
    // mouse_callback definition in the camera.h
    glfwSetCursorPosCallback(window, mouse_callback);
  }

  void terminate()
  {
    cleanUpVAOs();
    cleanUpTextures();
    glfwTerminate();
  }

  void resetViewport()
  {
    glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
  }

  void glFrameBufferClear()
  {
    // clean up last session
    glClearColor(clearColor.x, clearColor.y, clearColor.z, clearColor.w);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  }

  void run(const std::function<void(float)>&& draw)
  {
    while (!glfwWindowShouldClose(window))
    {
      // clean up last session
      glClearColor(clearColor.x, clearColor.y, clearColor.z, clearColor.w);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      handleInput();
      if(drawWorldCoord_){
        drawLine({1, 0, 0}, {-1, 0, 0}, {1, 0, 0, 1}, projectionMatrix_ * viewMatrix_);
        drawLine({0, 1, 0}, {0, -1, 0}, {0, 1, 0, 1}, projectionMatrix_ * viewMatrix_);
        drawLine({0, 0, 1}, {0, 0, -1}, {0, 0, 1, 1}, projectionMatrix_ * viewMatrix_);
      }

      if(mSkyBox_){ // bind cubemap texture and irr if available
        mSkyBox_->useCubemapTexture();
        mSkyBox_->useIBLTexture();
      }
      bindAllTextures();

      float t_delta = glfwGetTime() - lastFrame;
      draw(t_delta);
      lastFrame = glfwGetTime();

      // only take care the rotation and always render the skybox at last
      // because it should always have the least depth
      if(mSkyBox_) mSkyBox_->use(glm::mat4(glm::mat3(viewMatrix_)), projectionMatrix_);

      // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
      // -------------------------------------------------------------------------------
      glfwSwapBuffers(window);
      glfwPollEvents();
      // std::cout << "Frame rate: " << (int)(1/t_delta) << std::endl;
    }
  }

  // for processing input and set callback
  GLFWwindow *getWindow()
  {
    return window;
  }

  void setProjectionMatrix(const glm::mat4& proj)
  {
    projectionMatrix_ = proj;
  }

  void setProjectionMatrix(float fov, float near, float far)
  {
    projectionMatrix_ = glm::perspective(glm::radians(fov), SCR_WIDTH / (float)SCR_HEIGHT, near, far);
  }

  glm::mat4 getProjectionMatrix()
  {
    return projectionMatrix_;
  }

  void onViewMatrixUpdate(const glm::mat4& new_ViewMatrix, const glm::vec3& camPos) override
  {
    viewMatrix_ = new_ViewMatrix;
    cameraPos_ = camPos;
  }

  void useSkyBox(const std::string& name, unsigned int activeID, bool isEquirectangular=false, unsigned int irrID=0, unsigned int prefiltermap=0, unsigned int brdfLUT=0)
  {
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
    mSkyBox_ = std::make_unique<SkyBox>(name, activeID, isEquirectangular, irrID, prefiltermap, brdfLUT);
    if(isEquirectangular){
      resetViewport();
    }
  }

  void genFrameBuffer(unsigned int& framebuffer, unsigned int& textureColorbuffer, unsigned int& rbo)
  {
    // create the framebuffer for the window
    glGenFramebuffers(1, &framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    // create a color attachment texture, this is the color buffer of the window
    glGenTextures(1, &textureColorbuffer);
    glBindTexture(GL_TEXTURE_2D, textureColorbuffer);
    // set the color buffer size of the screen size,
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // attach the color buffer to the frame buffer
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColorbuffer, 0);
    // create a renderbuffer object for depth and stencil attachment (we won't be sampling these)
    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    // use a single renderbuffer object for both a depth AND stencil buffer. Set buffer size to the window size
    // depth buffer for each fragment will take 24 bits and the stencil takes 8 bits, in all it's 32 bits pre fragment
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, SCR_WIDTH, SCR_HEIGHT);
    // attach the depth_stencil render buffer to the framebuffer
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);
    // now that we actually created the framebuffer and added all attachments we want to check if it is actually complete now
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
    // reset the binded framebuffer to the default framebuffer created by glfw
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
  }

private:
  float lastFrame = 0.f;

  GLFWwindow *window;
  glm::vec3 cameraPos_ = glm::vec3(0);
  glm::mat4 viewMatrix_ = glm::mat4(1);
  glm::mat4 projectionMatrix_ = glm::mat4(1);

  std::unique_ptr<SkyBox> mSkyBox_ = nullptr;

  bool drawWorldCoord_ = false;
  // handle input control that is not related to camera
  // ---------------------------------------------------------------------------------------------
  void handleInput()
  {
    if(glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS) {
      // draw world coordinate
      drawWorldCoord_ = true;
    }
    if(glfwGetKey(window, GLFW_KEY_V) == GLFW_PRESS) {
      // erase world coordinate
      drawWorldCoord_ = false;
    }
  }
};

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
  // make sure the viewport matches the new window dimensions; note that width and
  // height will be significantly larger than specified on retina displays.
  glViewport(0, 0, width, height);
}