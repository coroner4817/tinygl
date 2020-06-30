#define STB_IMAGE_IMPLEMENTATION
#include "tinygl/stb_image.h"
#include <tinygl/utils.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <tinygl/shader_s.h>
#include <iostream>
#include <cmath>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/ext.hpp>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
// void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

unsigned int shaderID = -1;
float currentMixValue = 0.5f;

glm::vec3 cameraPos   = glm::vec3(0.0f, 0.0f,  3.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp    = glm::vec3(0.0f, 1.0f,  0.0f);
glm::vec3 cameraRight = glm::normalize(glm::cross(cameraUp, cameraFront));
glm::vec3 modelCenter = glm::vec3();
float deltaTime = 0.0f;	// Time between current frame and last frame
float lastFrame = 0.0f; // Time of last frame

// bool firstMouse = true;
// float lastX = 0, lastY = 0;
// float yaw = 0, pitch = 0;
float fov = 45.f;

int main()
{
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // uncomment this statement to fix compilation on OS X
#endif

  // glfw window creation
  // --------------------
  GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
  if (window == NULL)
  {
      std::cout << "Failed to create GLFW window" << std::endl;
      glfwTerminate();
      return -1;
  }
  glfwMakeContextCurrent(window);
  glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
  // glfwSetCursorPosCallback(window, mouse_callback);
  glfwSetScrollCallback(window, scroll_callback);

  // glad: load all OpenGL function pointers
  // ---------------------------------------
  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
  {
      std::cout << "Failed to initialize GLAD" << std::endl;
      return -1;
  }
  stbi_set_flip_vertically_on_load(true);

  // -------------------------------- begin of the actual program
  unsigned int texture1;
  glGenTextures(1, &texture1);
  glBindTexture(GL_TEXTURE_2D, texture1);

  // set the texture wrapping/filtering options (on the currently bound texture object)
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  int width, height, nrChannels;
  std::string texture_path = ASSETS_2D_BASEPATH + "/container.jpg";
  unsigned char *data = stbi_load(texture_path.c_str(), &width, &height, &nrChannels, 0);
  if (data){
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
  }else{
    std::cout << "Failed to load texture" << std::endl;
  }
  stbi_image_free(data);

  // use Texture Unit
  unsigned int texture2;
  glGenTextures(1, &texture2);
  glBindTexture(GL_TEXTURE_2D, texture2);
  // set the texture wrapping/filtering options (on the currently bound texture object)
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  std::string texture2_path = ASSETS_2D_BASEPATH + "/awesomeface.png";
  data = stbi_load(texture2_path.c_str(), &width, &height, &nrChannels, 0);
  if (data){
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
      glGenerateMipmap(GL_TEXTURE_2D);
  }else{
    std::cout << "Failed to load texture" << std::endl;
  }
  stbi_image_free(data);

  Shader ourShader(getExecutablePath() + "/shader.vs", getExecutablePath() + "/multitexture_shader.fs");
  shaderID = ourShader.ID;

  // tell opengl for each sampler to which texture unit it belongs to (only has to be done once)
  // -------------------------------------------------------------------------------------------
  ourShader.use(); // don't forget to activate/use the shader before setting uniforms!
  glUniform1i(glGetUniformLocation(ourShader.ID, "texture1"), 0);
  glUniform1i(glGetUniformLocation(ourShader.ID, "texture2"), 1);
  int mixvalueLocation = glGetUniformLocation(shaderID, "mixvalue");
  glUniform1f(mixvalueLocation, currentMixValue);

  // model transformation
  glm::mat4 model = glm::mat4(1.0f);
  model = glm::rotate(model, glm::radians(-55.0f), glm::vec3(1.0f, 0.0f, 0.0f));
  int matLoc = glGetUniformLocation(ourShader.ID, "model");
  glUniformMatrix4fv(matLoc, 1, GL_FALSE, glm::value_ptr(model));
  // camera matrix
  glm::mat4 view = glm::mat4(1.0f);
  // note that we're translating the scene in the reverse direction of where we want to move
  // since this is apply to the model not the camera, so we do it in the reverse way
  // camera is static, all the camera view transform is apply by transform the object in the inverse way
  view = glm::translate(view, glm::vec3(0.0f, 0.0f, -3.0f));
  matLoc = glGetUniformLocation(ourShader.ID, "view");
  glUniformMatrix4fv(matLoc, 1, GL_FALSE, glm::value_ptr(view));
  // projection matrix
  glm::mat4 projection;
  // fov and aspect ratio control how the camera view open this will affect the NDC mapping because the window open on NDC is fixed
  // near/far control how to culling the content along the z axis of the camera
  projection = glm::perspective(glm::radians(fov), SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
  matLoc = glGetUniformLocation(ourShader.ID, "projection");
  glUniformMatrix4fv(matLoc, 1, GL_FALSE, glm::value_ptr(projection));

  float vertices[] = {
    0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
    0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
    -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,

    0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
    0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
    -0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,

    -0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
    -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

    0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
    0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
    0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
    0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

    0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
    0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,

    0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
    0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
    -0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
    -0.5f,  0.5f, -0.5f,  0.0f, 1.0f
  };
  unsigned int indices[36];
  for(int i = 0; i < 6; ++i){
    indices[i*6 + 0] = 0 + 4 * i;
    indices[i*6 + 1] = 1 + 4 * i;
    indices[i*6 + 2] = 3 + 4 * i;
    indices[i*6 + 3] = 1 + 4 * i;
    indices[i*6 + 4] = 2 + 4 * i;
    indices[i*6 + 5] = 3 + 4 * i;
  }
  int vertexCnt = 36;

  glm::vec3 cubePositions[] = {
    glm::vec3( 0.0f,  0.0f,  0.0f),
    glm::vec3( 2.0f,  5.0f, -5.0f),
    glm::vec3(-1.5f, -2.2f, -2.5f),
    glm::vec3(-3.8f, -2.0f, -2.3f),
    glm::vec3( 2.4f, -0.4f, -3.5f),
    glm::vec3(-1.7f,  3.0f, -7.5f),
    glm::vec3( 1.3f, -2.0f, -2.5f),
    glm::vec3( 1.5f,  2.0f, -2.5f),
    glm::vec3( 1.5f,  0.2f, -1.5f),
    glm::vec3(-1.3f,  1.0f, -1.5f)
  };

  for(int i=0;i<10;++i){
    modelCenter += cubePositions[i];
  }
  modelCenter /= 10;

  glm::vec3 axis[10];
  for(int i=0;i<10;++i){
    axis[i] = {static_cast<float>(rand()) / static_cast<float>(RAND_MAX), static_cast<float>(rand()) / static_cast<float>(RAND_MAX), static_cast<float>(rand()) / static_cast<float>(RAND_MAX)};
  }

  unsigned int VBO, VAO, EBO;
  glGenVertexArrays(1, &VAO);
  glGenBuffers(1, &VBO);
  glGenBuffers(1, &EBO);
  // bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
  glBindVertexArray(VAO);
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

  // position attribute
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, (3+2) * sizeof(float), (void*)0);
  glEnableVertexAttribArray(0);
  // this location must match the location id in the vertex shader
  // texture attribute
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, (3+2) * sizeof(float), (void*)(3 * sizeof(float)));
  glEnableVertexAttribArray(2);

  // enable depth test when drawing
  glEnable(GL_DEPTH_TEST);
  // render loop
  // -----------
  while (!glfwWindowShouldClose(window))
  {
      float currentFrame = glfwGetTime();
      deltaTime = currentFrame - lastFrame;
      lastFrame = currentFrame;
      // input
      // -----
      processInput(window);

      // render
      // ------
      glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, texture1);
      glActiveTexture(GL_TEXTURE1);
      glBindTexture(GL_TEXTURE_2D, texture2);

      // render the triangle
      ourShader.use();
      float timeValue = glfwGetTime();
      float radius = std::sin(timeValue) / 2.0f + 0.5f;
      int radiusLocation = glGetUniformLocation(ourShader.ID, "radius");
      glUniform1f(radiusLocation, radius);

      glBindVertexArray(VAO);
      for(unsigned int i = 0; i < 10; i++) {
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, cubePositions[i]);
        model = glm::rotate(model, (float)glfwGetTime(), axis[i]);
        matLoc = glGetUniformLocation(ourShader.ID, "model");
        glUniformMatrix4fv(matLoc, 1, GL_FALSE, glm::value_ptr(model));

        // update the model transform which will update the camera view transform
        // float radius = 10.0f;
        // float camX = sin(glfwGetTime()) * radius;
        // float camZ = cos(glfwGetTime()) * radius;
        // glm::mat4 view;
        // view = glm::lookAt(glm::vec3(camX, 0, camZ) + modelCenter, modelCenter, glm::vec3(0.0, 1.0, 0.0));

        // control by wsad
        view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
        matLoc = glGetUniformLocation(ourShader.ID, "view");
        glUniformMatrix4fv(matLoc, 1, GL_FALSE, glm::value_ptr(view));

        projection = glm::perspective(glm::radians(fov), SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        matLoc = glGetUniformLocation(ourShader.ID, "projection");
        glUniformMatrix4fv(matLoc, 1, GL_FALSE, glm::value_ptr(projection));

        glDrawElements(GL_TRIANGLES, vertexCnt, GL_UNSIGNED_INT, 0);
      }

      // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
      // -------------------------------------------------------------------------------
      glfwSwapBuffers(window);
      glfwPollEvents();
  }

  // optional: de-allocate all resources once they've outlived their purpose:
  // ------------------------------------------------------------------------
  glDeleteVertexArrays(1, &VAO);
  glDeleteBuffers(1, &VBO);
  glDeleteBuffers(1, &EBO);

  glfwTerminate();
  return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window)
{
  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
      glfwSetWindowShouldClose(window, true);

  if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS){
    currentMixValue = currentMixValue + 0.005f;
    if(currentMixValue > 1.0f) currentMixValue = 1.0f;
    int mixvalueLocation = glGetUniformLocation(shaderID, "mixvalue");
    glUniform1f(mixvalueLocation, currentMixValue);
  }
  if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS){
    currentMixValue = currentMixValue - 0.005f;
    if(currentMixValue < 0.0f) currentMixValue = 0.0f;
    int mixvalueLocation = glGetUniformLocation(shaderID, "mixvalue");
    glUniform1f(mixvalueLocation, currentMixValue);
  }

  float cameraSpeed = 2.5f * deltaTime;
  if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    cameraPos += cameraSpeed * glm::normalize(cameraFront);
  if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    cameraPos -= cameraSpeed * glm::normalize(cameraFront);
  if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
  if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
  if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
    cameraPos -= glm::normalize(cameraUp) * cameraSpeed;
  if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
    cameraPos += glm::normalize(cameraUp) * cameraSpeed;

  // update 2 axis and the cameraPos
  if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS){
    // rotate alone Z axis, update up and right
    cameraUp = glm::rotate(cameraUp, cameraSpeed, glm::vec3(0, 0, 1));
    cameraPos = glm::rotate(cameraPos-modelCenter, cameraSpeed, glm::vec3(0, 0, 1)) + modelCenter;
  }
  if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS){
    // rotate alone X axis, update front and up
    cameraFront = glm::rotate(cameraFront, cameraSpeed, glm::vec3(1, 0, 0));
    cameraUp = glm::rotate(cameraUp, cameraSpeed, glm::vec3(1, 0, 0));
    cameraPos = glm::rotate(cameraPos-modelCenter, cameraSpeed, glm::vec3(1, 0, 0)) + modelCenter;
  }
  if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS){
    // rotate alone Y axis, update front and right
    cameraFront = glm::rotate(cameraFront, cameraSpeed, glm::vec3(0, 1, 0));
    cameraPos = glm::rotate(cameraPos-modelCenter, cameraSpeed, glm::vec3(0, 1, 0)) + modelCenter;
  }

  if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS){
    // reset
    cameraPos   = glm::vec3(0.0f, 0.0f,  3.0f);
    cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
    cameraUp    = glm::vec3(0.0f, 1.0f,  0.0f);
    cameraRight = glm::normalize(glm::cross(cameraUp, cameraFront));
  }
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

// void mouse_callback(GLFWwindow* window, double xpos, double ypos)
// {
//   if(firstMouse)
//   {
//     lastX = xpos;
//     lastY = ypos;
//     firstMouse = false;
//     return;
//   }

//   float xoffset = xpos - lastX;
//   float yoffset = lastY - ypos; // reversed since y-coordinates range from bottom to top
//   lastX = xpos;
//   lastY = ypos;

//   float sensitivity = 0.05f;
//   xoffset *= sensitivity;
//   yoffset *= sensitivity;

//   yaw   += xoffset;
//   pitch += yoffset;
//   if(pitch > 89.0f)
//   pitch =  89.0f;
//   if(pitch < -89.0f)
//     pitch = -89.0f;

//   glm::vec3 front;
//   front.x = cos(glm::radians(pitch)) * cos(glm::radians(yaw));
//   front.y = sin(glm::radians(pitch));
//   front.z = cos(glm::radians(pitch)) * sin(glm::radians(yaw));
//   cameraFront = glm::normalize(front);
//   // incomplete, still need to update up and pos
// }

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
  // equalivent to change cameraPos.z
  if(fov >= 1.0f && fov <= 45.0f)
  	fov -= yoffset;
  if(fov <= 1.0f)
  	fov = 1.0f;
  if(fov >= 45.0f)
  	fov = 45.0f;
}