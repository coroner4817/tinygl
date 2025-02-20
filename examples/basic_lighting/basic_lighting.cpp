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
void processInput(GLFWwindow *window);

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

unsigned int lightShaderID = -1;
float currentMixValue = 0.5f;

glm::vec3 lightPos(0.6f, 0.5f, 0.f);
glm::vec3 cameraPos(0, 0, 3);

/////////////////////////////////////////////////////////////////////
// Deprecated and have bug!!! Check the gltest for new implementation
/////////////////////////////////////////////////////////////////////
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

  Shader lightShader(getExecutablePath() + "/color.vs", getExecutablePath() + "/color.fs");
  lightShaderID = lightShader.ID;
  // tell opengl for each sampler to which texture unit it belongs to (only has to be done once)
  // -------------------------------------------------------------------------------------------
  lightShader.use(); // don't forget to activate/use the shader before setting uniforms!
  glUniform1i(glGetUniformLocation(lightShader.ID, "texture1"), 0);
  glUniform1i(glGetUniformLocation(lightShader.ID, "texture2"), 1);
  int mixvalueLocation = glGetUniformLocation(lightShaderID, "mixvalue");
  glUniform1f(mixvalueLocation, currentMixValue);
  int objColorLocation = glGetUniformLocation(lightShaderID, "objectColor");
  glUniform3f(objColorLocation, 1.f, 0.5f, 0.31f);
  int lightColorLocation = glGetUniformLocation(lightShaderID, "lightColor");
  glUniform3f(lightColorLocation, 1.f, 1.f, 1.f);
  int lightPosLocation = glGetUniformLocation(lightShaderID, "lightPos");
  glUniform3f(lightPosLocation, lightPos.x, lightPos.y, lightPos.z);

  // model transformation
  glm::mat4 model = glm::mat4(1.0f);
  model = glm::rotate(model, glm::radians(-55.0f), glm::vec3(1.0f, 0.0f, 0.0f));
  int matLoc = glGetUniformLocation(lightShader.ID, "model");
  glUniformMatrix4fv(matLoc, 1, GL_FALSE, glm::value_ptr(model));
  // camera matrix
  glm::mat4 view = glm::mat4(1.0f);
  // note that we're translating the scene in the reverse direction of where we want to move
  // since this is apply to the model not the camera, so we do it in the reverse way
  // camera is static, all the camera view transform is apply by transform the object in the inverse way
  view = glm::translate(view, -cameraPos);
  matLoc = glGetUniformLocation(lightShader.ID, "view");
  glUniformMatrix4fv(matLoc, 1, GL_FALSE, glm::value_ptr(view));
  // projection matrix
  glm::mat4 projection;
  // fov and aspect ratio control how the camera view open this will affect the NDC mapping because the window open on NDC is fixed
  // near/far control how to culling the content along the z axis of the camera
  projection = glm::perspective(glm::radians(45.0f), SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
  matLoc = glGetUniformLocation(lightShader.ID, "projection");
  glUniformMatrix4fv(matLoc, 1, GL_FALSE, glm::value_ptr(projection));

  int cameraPosLocation = glGetUniformLocation(lightShaderID, "cameraPos");
  glUniform3f(cameraPosLocation, cameraPos.x, cameraPos.y, cameraPos.z);

  // lamp shader
  Shader lampShader(getExecutablePath() + "/lampshader.vs", getExecutablePath() + "/lampshader.fs");
  lampShader.use();
  model = glm::mat4(1.0f);
  model = glm::translate(model, lightPos);
  model = glm::scale(model, glm::vec3(0.2f));
  matLoc = glGetUniformLocation(lampShader.ID, "model");
  glUniformMatrix4fv(matLoc, 1, GL_FALSE, glm::value_ptr(model));
  // here use the same view and projection matrix with the obj
  // so that this save us some trouble
  matLoc = glGetUniformLocation(lampShader.ID, "view");
  glUniformMatrix4fv(matLoc, 1, GL_FALSE, glm::value_ptr(view));
  matLoc = glGetUniformLocation(lampShader.ID, "projection");
  glUniformMatrix4fv(matLoc, 1, GL_FALSE, glm::value_ptr(projection));

  float vertices[] = {
    // vertex           // texture   // normal
    0.5f, -0.5f, -0.5f,  1.0f, 0.0f, 0.0f,  0.0f, -1.0f,
    0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 0.0f,  0.0f, -1.0f,
    -0.5f,  0.5f, -0.5f,  0.0f, 1.0f, 0.0f,  0.0f, -1.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, 0.0f, 0.0f,  0.0f, -1.0f,

    0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 0.0f,  0.0f, 1.0f,
    0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 0.0f,  0.0f, 1.0f,
    -0.5f,  0.5f,  0.5f,  0.0f, 1.0f, 0.0f,  0.0f, 1.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f, 0.0f,  0.0f, 1.0f,

    -0.5f,  0.5f, -0.5f,  1.0f, 1.0f, -1.0f,  0.0f,  0.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, 1.0f, -1.0f,  0.0f,  0.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f, -1.0f,  0.0f,  0.0f,
    -0.5f,  0.5f,  0.5f,  1.0f, 0.0f, -1.0f,  0.0f,  0.0f,

    0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 1.0f,  0.0f,  0.0f,
    0.5f, -0.5f, -0.5f,  0.0f, 1.0f, 1.0f,  0.0f,  0.0f,
    0.5f, -0.5f,  0.5f,  0.0f, 0.0f, 1.0f,  0.0f,  0.0f,
    0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 1.0f,  0.0f,  0.0f,

    0.5f, -0.5f, -0.5f,  1.0f, 1.0f, 0.0f, -1.0f,  0.0f,
    0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 0.0f, -1.0f,  0.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f, 0.0f, -1.0f,  0.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, 1.0f, 0.0f, -1.0f,  0.0f,

    0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 0.0f,  1.0f,  0.0f,
    0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 0.0f,  1.0f,  0.0f,
    -0.5f,  0.5f,  0.5f,  0.0f, 0.0f, 0.0f,  1.0f,  0.0f,
    -0.5f,  0.5f, -0.5f,  0.0f, 1.0f, 0.0f,  1.0f,  0.0f
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
    glm::vec3( 0.0f,  0.0f,  -5.0f),
    glm::vec3( 2.0f,  5.0f, -15.0f),
    glm::vec3(-1.5f, -1.f, -2.5f)
  };

  glm::vec3 axis[3];
  for(int i=0;i<3;++i){
    axis[i] = {static_cast<float>(rand()) / static_cast<float>(RAND_MAX), static_cast<float>(rand()) / static_cast<float>(RAND_MAX), static_cast<float>(rand()) / static_cast<float>(RAND_MAX)};
  }

  // obj cubes
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
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, (3+2+3) * sizeof(float), (void*)0);
  glEnableVertexAttribArray(0);
  // this location must match the location id in the vertex shader
  // texture attribute
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, (3+2+3) * sizeof(float), (void*)(3 * sizeof(float)));
  glEnableVertexAttribArray(2);

  glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, (3+2+3) * sizeof(float), (void*)(5 * sizeof(float)));
  glEnableVertexAttribArray(3);


  // create the lamp cube
  unsigned int lampVAO;
  glGenVertexArrays(1, &lampVAO);
  glBindVertexArray(lampVAO);
  // we only need to bind to the VBO, the container's VBO's data already contains the correct data.
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
  // set the vertex attributes (only position data for our lamp)
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, (3+2+3) * sizeof(float), (void*)0);
  glEnableVertexAttribArray(0);

  // enable depth test when drawing
  glEnable(GL_DEPTH_TEST);
  // render loop
  // -----------
  while (!glfwWindowShouldClose(window))
  {
      // input
      // -----
      processInput(window);

      // render
      // ------
      glClearColor(0.f, 0.f, 0.f, 1.0f);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, texture1);
      glActiveTexture(GL_TEXTURE1);
      glBindTexture(GL_TEXTURE_2D, texture2);

      lampShader.use();
      glBindVertexArray(lampVAO);
      // update the lamp position, rotate around the (0, 1, 0)
      glm::mat4 model = glm::mat4(1.0f);
      model = glm::translate(model, glm::rotate(lightPos, (float)glfwGetTime(), glm::vec3(0, 1, 0)));
      model = glm::scale(model, glm::vec3(0.2f));
      matLoc = glGetUniformLocation(lampShader.ID, "model");
      glUniformMatrix4fv(matLoc, 1, GL_FALSE, glm::value_ptr(model));

      glDrawElements(GL_TRIANGLES, vertexCnt, GL_UNSIGNED_INT, 0);
      glm::vec3 newLightPos = glm::vec3(model * glm::vec4(lightPos, 1.0f));

      // render the obj
      lightShader.use();
      float timeValue = glfwGetTime();
      float radius = std::sin(timeValue) / 2.0f + 0.5f;
      int radiusLocation = glGetUniformLocation(lightShader.ID, "radius");
      glUniform1f(radiusLocation, radius);

      lightPosLocation = glGetUniformLocation(lightShaderID, "lightPos");
      glUniform3f(lightPosLocation, newLightPos.x, newLightPos.y, newLightPos.z);

      glBindVertexArray(VAO);
      for(unsigned int i = 0; i < 3; i++) {
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, cubePositions[i]);
        // model = glm::rotate(model, (float)glfwGetTime(), axis[i]);
        matLoc = glGetUniformLocation(lightShader.ID, "model");
        glUniformMatrix4fv(matLoc, 1, GL_FALSE, glm::value_ptr(model));

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
    int mixvalueLocation = glGetUniformLocation(lightShaderID, "mixvalue");
    glUniform1f(mixvalueLocation, currentMixValue);
  }
  if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS){
    currentMixValue = currentMixValue - 0.005f;
    if(currentMixValue < 0.0f) currentMixValue = 0.0f;
    int mixvalueLocation = glGetUniformLocation(lightShaderID, "mixvalue");
    glUniform1f(mixvalueLocation, currentMixValue);
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