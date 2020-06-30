#include "tinygl/glApp.h"
#include "tinygl/camera.h"
#include "tinygl/shader_s.h"
#include "tinygl/default_mesh.h"

#include <map>
#include <ft2build.h>
#include FT_FREETYPE_H

// ifdef then will dynamically update the vertex buffer when rendering different character base on the character position
// ifndef then will use static vertex buffer and assign model transform matrix to each character
// #define DYNAMIC_BUF

/// Holds all state information relevant to a character as loaded using FreeType
struct Character {
    GLuint TextureID;   // ID handle of the glyph texture
    glm::ivec2 Size;    // Size of glyph
    glm::ivec2 Bearing;  // Offset from baseline to left/top of glyph
    GLuint Advance;    // Horizontal offset to advance to next glyph
};
std::map<GLchar, Character> Characters;

#ifdef DYNAMIC_BUF
// VAO for each character
GLuint VAO, VBO, EBO;
GLuint quad_indices[6] = {0, 1, 2, 0, 2, 3};
#endif

void RenderText(GLApp& app, DefaultMesh& dm, Shader &shader, std::string text, GLfloat x, GLfloat y, GLfloat scale, glm::vec3 color);

int main()
{
  GLApp app;
  app.init();
  // text rendering usually use orthographic projection
  glm::mat4 projection = glm::ortho(0.0f, (float)app.getScreenWidth(), 0.0f, (float)app.getScreenHeight());
  app.setProjectionMatrix(projection);

  DefaultMesh dm;
#ifndef DYNAMIC_BUF
  dm.initSpriteData(true);
  app.generateNewVAOFromArray("sprite", dm.getSpriteVerticesArray(), dm.getSpriteVerticesArraySize(), {{0, 4}}, dm.getSpriteIndicesArray(), dm.getSpriteIndicesArraySize());
#endif

  Shader textShader(COMMON_SHADERS_BASEPATH + "/sprite.vs", COMMON_SHADERS_BASEPATH + "/glyph.fs");
  textShader.init("model");
  textShader.setMatrix4fv("projection", projection);

  // only render the ccw face
  glEnable(GL_CULL_FACE);
  // due to we are rely on the alpha we need to enable blend
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  FT_Library ft;
  if (FT_Init_FreeType(&ft))
    std::cout << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;

  FT_Face face;
  if (FT_New_Face(ft,(ASSETS_BASEPATH + "font/Hack.ttf").c_str(), 0, &face))
    std::cout << "ERROR::FREETYPE: Failed to load font" << std::endl;

  // 0 set load glyph with dynamic resolution
  FT_Set_Pixel_Sizes(face, 0, 48);

  // Disable byte-alignment restriction
  // so thatce we can pass the grayscale 8-bit texture
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

  for (GLubyte c = 0; c < 128; c++)
  {
      // Load character glyph
      if (FT_Load_Char(face, c, FT_LOAD_RENDER))
      {
          std::cout << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;
          continue;
      }
      // Generate texture
      GLuint texture;
      glGenTextures(1, &texture);
      glBindTexture(GL_TEXTURE_2D, texture);
      glTexImage2D(
          GL_TEXTURE_2D,
          0,
          GL_RED, // red becuase the generated text bitmap is a single channel gray scale
          face->glyph->bitmap.width,
          face->glyph->bitmap.rows,
          0,
          GL_RED,
          GL_UNSIGNED_BYTE,
          face->glyph->bitmap.buffer
      );
      // Set texture options
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      // Now store character for later use
      Character character = {
          .TextureID = texture,
          .Size = glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
          .Bearing = glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
          .Advance = face->glyph->advance.x
      };
      Characters.insert(std::pair<GLchar, Character>(c, character));
  }
  glBindTexture(GL_TEXTURE_2D, 0);
  // Destroy FreeType once we're finished
  // the texture id are stored in the map
  FT_Done_Face(face);
  FT_Done_FreeType(ft);

#ifdef DYNAMIC_BUF
  // Configure VAO/VBO for texture quads
  // the VAO will be reused for all character
  // for each character we need 6 vertices(no reuse) and each of vertex need 4 float <pos> <tex_pos>
  glGenVertexArrays(1, &VAO);
  glGenBuffers(1, &VBO);
  glBindVertexArray(VAO);
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  // Since we are going to reuse, so the data pointer passed here is NULL
  // so that when we filling data, we need to use the glBufferSubData as following
  glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 4 * 4, NULL, GL_DYNAMIC_DRAW);
  // EBO is fixed so not dynamic
  glGenBuffers(1, &EBO);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(quad_indices), &quad_indices, GL_STATIC_DRAW);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), 0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);
#endif

  app.run([=, &app, &textShader, &dm](float a_delta){
    RenderText(app, dm, textShader, "This is sample text", 25.0f, 25.0f, 1.0f, glm::vec3(0.5, 0.8f, 0.2f));
    RenderText(app, dm, textShader, "(C) LearnOpenGL.com", 540.0f, 570.0f, 0.5f, glm::vec3(0.3, 0.7f, 0.9f));
  });
}

void RenderText(GLApp& app, DefaultMesh& dm, Shader &shader, std::string text, GLfloat x, GLfloat y, GLfloat scale, glm::vec3 color)
{
    // Activate corresponding render state
    shader.use();
    shader.setVec3("textColor", color);

    // need to manage the texture activation here because we are not generate the texture through our texture manager
    glActiveTexture(GL_TEXTURE0);

    // Iterate through all characters
    std::string::const_iterator c;
    for (c = text.begin(); c != text.end(); c++)
    {
        Character ch = Characters[*c];

        // x, y are iterated updated
        GLfloat xpos = x + ch.Bearing.x * scale;
        GLfloat ypos = y - (ch.Size.y - ch.Bearing.y) * scale;

        GLfloat w = ch.Size.x * scale;
        GLfloat h = ch.Size.y * scale;

#ifdef DYNAMIC_BUF
        shader.setModelMatrix(); // set default
        glBindVertexArray(VAO);
        // Update VBO for each character
        GLfloat vertices[4][4] = {
            // the vertex position is in CCW
            // the texture vertice is not matched becuase the texture origin is at top left
            // the is because freetype doesn't provide a flag to load the glyph flip vertically
            { xpos,     ypos + h,   0.0, 0.0 },
            { xpos,     ypos,       0.0, 1.0 },
            { xpos + w, ypos,       1.0, 1.0 },
            { xpos + w, ypos + h,   1.0, 0.0 }
        };
        // Update content of VBO memory
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        // use sub buffer because we are reuse the same peices of memory
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices); // Be sure to use glBufferSubData and not glBufferData
        // Render quad
        glBindVertexArray(VAO);
#else
        app.useVAO("sprite");
        shader.setModelMatrix({xpos, ypos, 0}, {w, h, 1});
#endif
        // Render glyph texture over quad
        glBindTexture(GL_TEXTURE_2D, ch.TextureID);

        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        // update x. notice that if multilines we also need to update y
        // Now advance cursors for next glyph (DEFINITION: note that advance is number of 1/64 pixels)
        x += (ch.Advance >> 6) * scale; // Bitshift by 6 to get value in pixels (2^6 = 64 (divide amount of 1/64th pixels by 64 to get amount of pixels))
    }
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}