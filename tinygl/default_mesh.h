#pragma once
#include "tinygl/common.h"
#include "tinygl/glApp.h"
#include "tinygl/camera.h"
#include "tinygl/shader_s.h"

class GLApp;
class Camera;
class Shader;
class DefaultMesh : public VAOManager
{
GLApp* app_;
Camera* cam_;
Shader* stdShader_ = nullptr;
Shader* sphereShader_ = nullptr;

bool objAttrCompatible_ = false;
public:
  DefaultMesh(){};
  DefaultMesh(GLApp* app, Camera* cam){
    app_ = app;
    cam_ = cam;
  }
  ~DefaultMesh()
  {
    if (SPHERE_VERTICES)
      delete[] SPHERE_VERTICES;
    if (SPHERE_INDICES)
      delete[] SPHERE_INDICES;

    if(stdShader_)
      delete stdShader_;

    if(sphereShader_)
      delete sphereShader_;

    cleanUpVAOs();
  }
  void setObjAttrComp(bool b) { objAttrCompatible_ = b; }
  void initStdShader()
  {
    if(!stdShader_){
      stdShader_ = new Shader(COMMON_SHADERS_BASEPATH + "/stdShader.vs", COMMON_SHADERS_BASEPATH + "/stdShader.fs");
      stdShader_->init("model", "view", cam_, "projection", app_->getProjectionMatrix());
    }
  }

  void initSphereShader()
  {
    if(!sphereShader_){
      sphereShader_ = new Shader(COMMON_SHADERS_BASEPATH + "/sphere.vs", COMMON_SHADERS_BASEPATH + "/sphere.fs");
      sphereShader_->init("model", "view", cam_, "projection", app_->getProjectionMatrix());
    }
  }

  void initSphere()
  {
    initSphereData();
    initSphereShader();
    generateNewVAOFromArray("sphere", getSphereVerticesArray(), getSphereVerticesArraySize(), {{0, 3}}, getSphereIndicesArray(), getSphereIndicesArraySize());
  }

  void initCube()
  {
    initCubeData();
    initStdShader();
    generateNewVAOFromArray("cube", getCubeVerticesArray(), getCubeVerticesArraySize(), {{0, 3}, {1, 2}, {2, 3}}, getCubeIndicesArray(), getCubeIndicesArraySize());
  }

  void initPlane()
  {
    initPlaneData();
    initStdShader();
    generateNewVAOFromArray("plane", getPlaneVerticesArray(), getPlaneVerticesArraySize(), {{0, 3}, {1, 2}, {2, 3}}, getPlaneIndicesArray(), getPlaneIndicesArraySize());  }

  // ---------------------------------------------------
  // Cube func
  void initCubeData()
  {
    for (int i = 0; i < 6; ++i)
    {
      CUBE_INDICES[i * 6 + 0] = 0 + 4 * i;
      CUBE_INDICES[i * 6 + 1] = 1 + 4 * i;
      CUBE_INDICES[i * 6 + 2] = 3 + 4 * i;
      CUBE_INDICES[i * 6 + 3] = 1 + 4 * i;
      CUBE_INDICES[i * 6 + 4] = 2 + 4 * i;
      CUBE_INDICES[i * 6 + 5] = 3 + 4 * i;
    }
  }

  void drawCube(unsigned int texture1, glm::vec3 pos, glm::vec3 scl=glm::vec3(1), float angle=0, glm::vec3 axis=glm::vec3())
  {
    if(stdShader_){
      stdShader_->use();
      useVAO("cube");
      stdShader_->setInt("isColor", false);
      stdShader_->setInt("texture1", texture1);
      stdShader_->setModelMatrix(pos, scl, angle, axis);
      glDrawElements(GL_TRIANGLES, getCubeVerticesCount(), GL_UNSIGNED_INT, 0);
    }
  }

  void drawCube(glm::vec3 color, glm::vec3 pos, glm::vec3 scl=glm::vec3(1), float angle=0, glm::vec3 axis=glm::vec3())
  {
    if(stdShader_){
      stdShader_->use();
      useVAO("cube");
      stdShader_->setInt("isColor", true);
      stdShader_->setVec3("color", color);
      stdShader_->setModelMatrix(pos, scl, angle, axis);
      glDrawElements(GL_TRIANGLES, getCubeVerticesCount(), GL_UNSIGNED_INT, 0);
    }
  }

  const void *getCubeVerticesArray()
  {
    if(objAttrCompatible_) return static_cast<const void *>(CUBE_VERTICES_OBJ);
    return static_cast<const void *>(CUBE_VERTICES);
  }

  int getCubeVerticesArraySize()
  {
    return sizeof(CUBE_VERTICES);
  }

  const void *getCubeIndicesArray()
  {
    return static_cast<const void *>(CUBE_INDICES);
  }

  int getCubeIndicesArraySize()
  {
    return sizeof(CUBE_INDICES);
  }
  int getCubeVerticesCount()
  {
    return CUBE_VERTICES_CNT;
  }

  // ---------------------------------------------------
  // plane func
  void initPlaneData()
  {
    for(int i = 0; i < 6; ++i){
      PLANE_INDICES[i] = i;
    }
  }

  void drawPlane(unsigned int texture1, glm::vec3 pos, glm::vec3 scl=glm::vec3(1), float angle=0, glm::vec3 axis=glm::vec3())
  {
    if(stdShader_){
      stdShader_->use();
      useVAO("plane");
      stdShader_->setInt("isColor", false);
      stdShader_->setInt("texture1", texture1);
      stdShader_->setModelMatrix(pos, scl, angle, axis);
      glDrawElements(GL_TRIANGLES, getPlaneVerticesCount(), GL_UNSIGNED_INT, 0);
    }
  }

  void drawPlane(glm::vec3 color, glm::vec3 pos, glm::vec3 scl=glm::vec3(1), float angle=0, glm::vec3 axis=glm::vec3())
  {
    if(stdShader_){
      stdShader_->use();
      useVAO("plane");
      stdShader_->setInt("isColor", true);
      stdShader_->setVec3("color", color);
      stdShader_->setModelMatrix(pos, scl, angle, axis);
      glDrawElements(GL_TRIANGLES, getPlaneVerticesCount(), GL_UNSIGNED_INT, 0);
    }
  }

  const void* getPlaneVerticesArray()
  {
    if(objAttrCompatible_) return static_cast<const void *>(PLANE_VERTICES_OBJ);
    return static_cast<const void*>(PLANE_VERTICES);
  }

  int getPlaneVerticesArraySize()
  {
    return sizeof(PLANE_VERTICES);
  }

  const void* getPlaneIndicesArray()
  {
    return static_cast<const void*>(PLANE_INDICES);
  }

  int getPlaneIndicesArraySize()
  {
    return sizeof(PLANE_INDICES);
  }

  int getPlaneVerticesCount()
  {
    return PLANE_VERTICES_CNT;
  }

  // ---------------------------------------------------
  // quad func
  void initQuadData()
  {
  }

  const void* getQuadVerticesArray()
  {
    return static_cast<const void*>(QUAD_VERTICES);
  }

  int getQuadVerticesArraySize()
  {
    return sizeof(QUAD_VERTICES);
  }

  const void* getQuadIndicesArray()
  {
    return static_cast<const void*>(QUAD_INDICES);
  }

  int getQuadIndicesArraySize()
  {
    return sizeof(QUAD_INDICES);
  }

  int getQuadVerticesCount()
  {
    return QUAD_VERTICES_CNT;
  }

  // ---------------------------------------------------
  // sprite func
  void initSpriteData(bool flip=false)
  {
    sprite_flip_ = flip;
  }

  const void* getSpriteVerticesArray()
  {
    return sprite_flip_ ? static_cast<const void*>(SPRITE_VERTICES_FLIP) : static_cast<const void*>(SPRITE_VERTICES);
  }

  int getSpriteVerticesArraySize()
  {
    return sizeof(SPRITE_VERTICES);
  }

  const void* getSpriteIndicesArray()
  {
    return static_cast<const void*>(SPRITE_INDICES);
  }

  int getSpriteIndicesArraySize()
  {
    return sizeof(SPRITE_INDICES);
  }

  int getSpriteVerticesCount()
  {
    return SPRITE_VERTICES_CNT;
  }

  // ---------------------------------------------------
  // sphere func
  void drawSphere(glm::vec4 color, glm::vec3 pos, glm::vec3 scl=glm::vec3(1), float angle=0, glm::vec3 axis=glm::vec3())
  {
    if(sphereShader_){
      sphereShader_->use();
      useVAO("sphere");
      sphereShader_->setVec4("color", color);
      sphereShader_->setModelMatrix(pos, scl, angle, axis);
      glDrawElements(GL_TRIANGLES, getSphereVerticesCount(), GL_UNSIGNED_INT, 0);
    }
  }

  void dumpSphereVertex(const glm::vec3 &p)
  {
    SPHERE_VERTICES[mVertexCount * 3] = p.x;
    SPHERE_VERTICES[mVertexCount * 3 + 1] = p.y;
    SPHERE_VERTICES[mVertexCount * 3 + 2] = p.z;
    mVertexCount++;
  }

  void dumpSphereVertexUvNorm(const glm::vec3 &p, const glm::vec2& uv)
  {
    // vertex
    SPHERE_VERTICES[mVertexCount * 8] = p.x;
    SPHERE_VERTICES[mVertexCount * 8 + 1] = p.y;
    SPHERE_VERTICES[mVertexCount * 8 + 2] = p.z;
    // uv
    SPHERE_VERTICES[mVertexCount * 8 + 3] = uv.x;
    SPHERE_VERTICES[mVertexCount * 8 + 4] = uv.y;
    // normal
    SPHERE_VERTICES[mVertexCount * 8 + 5] = p.x;
    SPHERE_VERTICES[mVertexCount * 8 + 6] = p.y;
    SPHERE_VERTICES[mVertexCount * 8 + 7] = p.z;
    mVertexCount++;
  }

  void dumpSphereTriangle(const glm::vec3 &p1, const glm::vec3 &p2, const glm::vec3 &p3, bool uvnormal=false, const std::vector<glm::vec2>& uvs={})
  {
    if(!uvnormal){
      dumpSphereVertex(p1);
      dumpSphereVertex(p2);
      dumpSphereVertex(p3);
    }else{
      dumpSphereVertexUvNorm(p1, uvs[0]);
      dumpSphereVertexUvNorm(p2, uvs[1]);
      dumpSphereVertexUvNorm(p3, uvs[2]);
    }
  }

  // FIXME: uvnormal generation not working
  void initSphereData(float radius=1, int stacks=15, int sectors=15, bool uvnormal=false)
  {
    mVertexCount = 0;
    SPHERE_VERTICES_CNT = 0;
    SPHERE_INDICES_CNT = 0;
    if (SPHERE_VERTICES)
      delete[] SPHERE_VERTICES;
    if (SPHERE_INDICES)
      delete[] SPHERE_INDICES;
    float PI = 3.1415926;

    if (stacks % 2 == 0)
      stacks -= 1;
    float half_sphere_stacks = (stacks - 1) / 2;

    // assume the origin will be at (0, 0, 0)
    glm::vec3 origin = glm::vec3();

    // size_t total_vertices = 2 + sectors * stacks;

    std::vector<glm::vec3> vertices;
    vertices.push_back(origin + glm::vec3(0, radius, 0));

    // only generate the half of the sphere and mirror to get the other side
    for (int i = 0; i < half_sphere_stacks; ++i)
    {
      float phi = PI / 2 / (half_sphere_stacks + 1) * (half_sphere_stacks - i);

      for (int j = 0; j < sectors; ++j)
      {
        float theta = 2 * PI / (float)sectors * j;

        vertices.push_back(glm::vec3(
            radius * glm::cos(phi) * glm::sin(theta),
            radius * glm::sin(phi),
            radius * glm::cos(phi) * glm::cos(theta)));
      }
    }

    std::vector<glm::vec3> other_half = vertices;
    for (auto &v : other_half)
    {
      v = glm::vec3(v.x, -v.y, v.z);
    }
    std::reverse(std::begin(other_half), std::end(other_half));
    for (int i = 0; i < half_sphere_stacks; ++i)
    {
      std::reverse(std::begin(other_half) + i * sectors, std::begin(other_half) + (i + 1) * sectors);
    }

    // push the middle stack first
    for (int i = 0; i < sectors; ++i)
    {
      float theta = 2 * PI / (float)sectors * i;
      vertices.push_back(glm::vec3(
          radius * glm::sin(theta),
          0,
          radius * glm::cos(theta)));
    }

    vertices.insert(vertices.end(), other_half.begin(), other_half.end());
    SPHERE_VERTICES_CNT = (sectors + (stacks - 1) * sectors * 2 + (int)vertices.size() - (1 + sectors * (stacks - 1))) * 3 * 3;
    if(uvnormal) SPHERE_VERTICES_CNT = SPHERE_VERTICES_CNT / 3 * 8;
    SPHERE_VERTICES = new float[SPHERE_VERTICES_CNT];

    // dump top area vertices
    for (int i = 1; i < sectors + 1; ++i)
    {
      if (i == sectors)
      {
        dumpSphereTriangle(vertices[0], vertices[i], vertices[1], uvnormal, {{0.5, 1}, {(i-1)/sectors, 1-1/stacks}, {1, 1-1/stacks}});
        break;
      }
      dumpSphereTriangle(vertices[0], vertices[i], vertices[i + 1], uvnormal, {{0.5, 1}, {(i-1)/sectors, 1-1/stacks}, {i/sectors, 1-1/stacks}});
    }

    // dump triangle strip till the bottom point
    size_t cur_st, prev_st;
    for (int i = 0; i < (stacks - 1); ++i)
    {
      cur_st = 1 + sectors * (i + 1);
      prev_st = 1 + sectors * i;

      for (int j = 0; j < sectors; ++j)
      {
        if (j == sectors - 1)
        {
          dumpSphereTriangle(vertices[cur_st + j], vertices[prev_st + j], vertices[cur_st], uvnormal, {{j/sectors, 1-(i+2)/stacks}, {j/sectors, 1-(i+1)/stacks}, {1, 1-(i+2)/stacks}});
          dumpSphereTriangle(vertices[prev_st + j], vertices[prev_st], vertices[cur_st], uvnormal, {{j/sectors, 1-(i+1)/stacks}, {1, 1-(i+1)/stacks}, {1, 1-(i+2)/stacks}});
          break;
        }
        dumpSphereTriangle(vertices[cur_st + j], vertices[prev_st + j], vertices[cur_st + j + 1], uvnormal, {{j/sectors, 1-(i+2)/stacks}, {j/sectors, 1-(i+1)/stacks}, {(j+1)/sectors, 1-(i+2)/stacks}});
        dumpSphereTriangle(vertices[prev_st + j], vertices[prev_st + j + 1], vertices[cur_st + j + 1], uvnormal, {{j/sectors, 1-(i+1)/stacks}, {(j+1)/sectors, 1-(i+1)/stacks}, {(j+1)/sectors, 1-(i+2)/stacks}});
      }
    }

    // handle the bottom point
    int bottom_begin = 1 + sectors * (stacks - 1);
    for (int i = bottom_begin; i < (int)vertices.size(); ++i)
    {
      if (i == (int)vertices.size() - 1)
      {
        dumpSphereTriangle(vertices[i], vertices.back(), vertices[bottom_begin], uvnormal, {{(i-bottom_begin)/sectors, 1/stacks}, {0.5, 0}, {1, 1/stacks}});
        break;
      }
      dumpSphereTriangle(vertices[i], vertices[i + 1], vertices.back(), uvnormal, {{(i-bottom_begin)/sectors, 1/stacks}, {(i+1-bottom_begin)/sectors, 1/stacks}, {0.5, 0}});
    }

    // indices
    SPHERE_INDICES_CNT = SPHERE_VERTICES_CNT;
    if(uvnormal) SPHERE_INDICES_CNT = SPHERE_INDICES_CNT / 8 * 3;
    SPHERE_INDICES = new unsigned int[SPHERE_INDICES_CNT];
    for(int i = 0;i<SPHERE_INDICES_CNT;++i){
      SPHERE_INDICES[i] = i;
    }
  }

  // has to use glDrawElements(GL_TRIANGLE_STRIP)
  void initSphereData2()
  {
    std::vector<glm::vec3> positions;
    std::vector<glm::vec2> uv;
    std::vector<glm::vec3> normals;
    std::vector<unsigned int> indices;

    const unsigned int X_SEGMENTS = 64;
    const unsigned int Y_SEGMENTS = 64;
    const float PI = 3.14159265359;
    for (unsigned int y = 0; y <= Y_SEGMENTS; ++y)
    {
        for (unsigned int x = 0; x <= X_SEGMENTS; ++x)
        {
            float xSegment = (float)x / (float)X_SEGMENTS;
            float ySegment = (float)y / (float)Y_SEGMENTS;
            float xPos = std::cos(xSegment * 2.0f * PI) * std::sin(ySegment * PI);
            float yPos = std::cos(ySegment * PI);
            float zPos = std::sin(xSegment * 2.0f * PI) * std::sin(ySegment * PI);

            positions.push_back(glm::vec3(xPos, yPos, zPos));
            uv.push_back(glm::vec2(xSegment, ySegment));
            normals.push_back(glm::vec3(xPos, yPos, zPos));
        }
    }

    bool oddRow = false;
    for (int y = 0; y < Y_SEGMENTS; ++y)
    {
        if (!oddRow) // even rows: y == 0, y == 2; and so on
        {
            for (int x = 0; x <= X_SEGMENTS; ++x)
            {
                indices.push_back(y       * (X_SEGMENTS + 1) + x);
                indices.push_back((y + 1) * (X_SEGMENTS + 1) + x);
            }
        }
        else
        {
            for (int x = X_SEGMENTS; x >= 0; --x)
            {
                indices.push_back((y + 1) * (X_SEGMENTS + 1) + x);
                indices.push_back(y       * (X_SEGMENTS + 1) + x);
            }
        }
        oddRow = !oddRow;
    }
    SPHERE_INDICES_CNT = indices.size();
    SPHERE_INDICES = new unsigned int[SPHERE_INDICES_CNT];
    std::memcpy(SPHERE_INDICES, &indices[0], SPHERE_INDICES_CNT * sizeof(unsigned int));

    std::vector<float> data;
    for (int i = 0; i < positions.size(); ++i)
    {
        data.push_back(positions[i].x);
        data.push_back(positions[i].y);
        data.push_back(positions[i].z);
        if (uv.size() > 0)
        {
            data.push_back(uv[i].x);
            data.push_back(uv[i].y);
        }
        if (normals.size() > 0)
        {
            data.push_back(normals[i].x);
            data.push_back(normals[i].y);
            data.push_back(normals[i].z);
        }
    }
    SPHERE_VERTICES_CNT = data.size();
    SPHERE_VERTICES = new float[SPHERE_VERTICES_CNT];
    std::memcpy(SPHERE_VERTICES, &data[0], SPHERE_VERTICES_CNT * sizeof(float));
  }

  const void *getSphereVerticesArray()
  {
    return static_cast<const void *>(SPHERE_VERTICES);
  }

  int getSphereVerticesArraySize()
  {
    // assume only have vertices in the array
    return SPHERE_VERTICES_CNT * sizeof(float);
  }

  const void *getSphereIndicesArray()
  {
    return static_cast<const void *>(SPHERE_INDICES);
  }

  int getSphereIndicesArraySize()
  {
    return SPHERE_INDICES_CNT * sizeof(unsigned int);
  }
  int getSphereVerticesCount()
  {
    return SPHERE_INDICES_CNT;
  }

private:
  // ---------------------------------------------------
  // Cube data
  static const int CUBE_VERTICES_CNT = 36;
  const float CUBE_VERTICES[4 * 6 * 8] = {
      // vertex           // texture        // normal
      0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 0.0f, -1.0f,
      0.5f, 0.5f, -0.5f, 1.0f, 1.0f, 0.0f, 0.0f, -1.0f,
      -0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, -1.0f,
      -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f,

      0.5f, -0.5f, 0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
      0.5f, 0.5f, 0.5f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
      -0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
      -0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f,

      -0.5f, 0.5f, -0.5f, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f,
      -0.5f, -0.5f, -0.5f, 0.0f, 1.0f, -1.0f, 0.0f, 0.0f,
      -0.5f, -0.5f, 0.5f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f,
      -0.5f, 0.5f, 0.5f, 1.0f, 0.0f, -1.0f, 0.0f, 0.0f,

      0.5f, 0.5f, -0.5f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f,
      0.5f, -0.5f, -0.5f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f,
      0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
      0.5f, 0.5f, 0.5f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f,

      0.5f, -0.5f, -0.5f, 1.0f, 1.0f, 0.0f, -1.0f, 0.0f,
      0.5f, -0.5f, 0.5f, 1.0f, 0.0f, 0.0f, -1.0f, 0.0f,
      -0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f,
      -0.5f, -0.5f, -0.5f, 0.0f, 1.0f, 0.0f, -1.0f, 0.0f,

      0.5f, 0.5f, -0.5f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f,
      0.5f, 0.5f, 0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
      -0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f,
      -0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f
  };
  const float CUBE_VERTICES_OBJ[4 * 6 * 8] = {
      // vertex                 // normal,             // texture
      0.5f, -0.5f, -0.5f,     0.0f, 0.0f, -1.0,    1.0f, 0.0f,
      0.5f, 0.5f, -0.5f,      0.0f, 0.0f, -1.0,    1.0f, 1.0f,
      -0.5f, 0.5f, -0.5f,     0.0f, 0.0f, -1.0,    0.0f, 1.0f,
      -0.5f, -0.5f, -0.5f,    0.0f, 0.0f, -1.0,    0.0f, 0.0f,

      0.5f, -0.5f, 0.5f,      0.0f, 0.0f, 1.0f,    1.0f, 0.0f,
      0.5f, 0.5f, 0.5f,       0.0f, 0.0f, 1.0f,    1.0f, 1.0f,
      -0.5f, 0.5f, 0.5f,      0.0f, 0.0f, 1.0f,    0.0f, 1.0f,
      -0.5f, -0.5f, 0.5f,     0.0f, 0.0f, 1.0f,    0.0f, 0.0f,

      -0.5f, 0.5f, -0.5f,     -1.0f, 0.0f, 0.0,    1.0f, 1.0f,
      -0.5f, -0.5f, -0.5f,    -1.0f, 0.0f, 0.0,    0.0f, 1.0f,
      -0.5f, -0.5f, 0.5f,     -1.0f, 0.0f, 0.0,    0.0f, 0.0f,
      -0.5f, 0.5f, 0.5f,      -1.0f, 0.0f, 0.0,    1.0f, 0.0f,

      0.5f, 0.5f, -0.5f,      1.0f, 0.0f, 0.0f,    1.0f, 1.0f,
      0.5f, -0.5f, -0.5f,     1.0f, 0.0f, 0.0f,    0.0f, 1.0f,
      0.5f, -0.5f, 0.5f,      1.0f, 0.0f, 0.0f,    0.0f, 0.0f,
      0.5f, 0.5f, 0.5f,       1.0f, 0.0f, 0.0f,    1.0f, 0.0f,

      0.5f, -0.5f, -0.5f,     0.0f, -1.0f, 0.0,    1.0f, 1.0f,
      0.5f, -0.5f, 0.5f,      0.0f, -1.0f, 0.0,    1.0f, 0.0f,
      -0.5f, -0.5f, 0.5f,     0.0f, -1.0f, 0.0,    0.0f, 0.0f,
      -0.5f, -0.5f, -0.5f,    0.0f, -1.0f, 0.0,    0.0f, 1.0f,

      0.5f, 0.5f, -0.5f,      0.0f, 1.0f, 0.0f,    1.0f, 1.0f,
      0.5f, 0.5f, 0.5f,       0.0f, 1.0f, 0.0f,    1.0f, 0.0f,
      -0.5f, 0.5f, 0.5f,      0.0f, 1.0f, 0.0f,    0.0f, 0.0f,
      -0.5f, 0.5f, -0.5f,     0.0f, 1.0f, 0.0f,    0.0f, 1.0f,
  };
  unsigned int CUBE_INDICES[CUBE_VERTICES_CNT];

  // ---------------------------------------------------
  // plane data
  static const int PLANE_VERTICES_CNT = 6;
  const float PLANE_VERTICES[6 * 8] = {
    // vertex           // texture        // normal
    5.0f, 0.0f,  5.0f,  5.0f, 0.0f, 0.0f, 1.0f, 0.0f,
    -5.0f, 0.0f,  5.0f,  0.0f, 0.0f, 0.0f, 1.0f, 0.0f,
    -5.0f, 0.0f, -5.0f,  0.0f, 5.0f, 0.0f, 1.0f, 0.0f,

    5.0f, 0.0f,  5.0f,  5.0f, 0.0f, 0.0f, 1.0f, 0.0f,
    -5.0f, 0.0f, -5.0f,  0.0f, 5.0f, 0.0f, 1.0f, 0.0f,
    5.0f, 0.0f, -5.0f,  5.0f, 5.0f, 0.0f, 1.0f, 0.0f
  };
  const float PLANE_VERTICES_OBJ[6 * 8] = {
    // vertex              // normal            // texture
    5.0f, 0.0f,  5.0f,     0.0f, 1.0f, 0.0f,     5.0f, 0.0f,
    -5.0f, 0.0f,  5.0f,    0.0f, 1.0f, 0.0f,    0.0f, 0.0f,
    -5.0f, 0.0f, -5.0f,    0.0f, 1.0f, 0.0f,    0.0f, 5.0f,

    5.0f, 0.0f,  5.0f,     0.0f, 1.0f, 0.0f,     5.0f, 0.0f,
    -5.0f, 0.0f, -5.0f,    0.0f, 1.0f, 0.0f,    0.0f, 5.0f,
    5.0f, 0.0f, -5.0f,     0.0f,  1.0f, 0.0f,   5.0f, 5.0f,
  };
  unsigned int PLANE_INDICES[PLANE_VERTICES_CNT] = {
    0, 1, 2,
    3, 4, 5
  };

  // ---------------------------------------------------
  // NDC quad
  // vertex attributes for a quad that fills the entire screen in Normalized Device Coordinates.
  static const int QUAD_VERTICES_CNT = 6;
  const float QUAD_VERTICES[4 * 5] = {
    // positions        // texCoords
    -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
    -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
      1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
      1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
  };
  const unsigned int QUAD_INDICES[QUAD_VERTICES_CNT] = {
    0, 1, 2,
    1, 2, 3
  };

  // ---------------------------------------------------
  // Sprite quad
  // the only different with the NDC quad is that we only pass 2D coordinate
  // NOTICE: if set to ture then we will flip the vertices vertically
  // this is only use for freetype becuase it doesn't provide a flag to stbi_set_flip_vertically_on_load(true);
  bool sprite_flip_ = false;
  static const int SPRITE_VERTICES_CNT = 6;
  const float SPRITE_VERTICES[4 * 4] = {
      // positions        // texCoords
      0.0, 1.0, 0.0, 1.0,
      0.0, 0.0, 0.0, 0.0,
      1.0, 0.0, 1.0, 0.0,
      1.0, 1.0, 1.0, 1.0,
    };
    const float SPRITE_VERTICES_FLIP[4 * 4] = {
      // positions        // texCoords
      0.0, 1.0, 0.0, 0.0,
      0.0, 0.0, 0.0, 1.0,
      1.0, 0.0, 1.0, 1.0,
      1.0, 1.0, 1.0, 0.0,
    };
    const unsigned int SPRITE_INDICES[SPRITE_VERTICES_CNT] = {
      0, 1, 2,
      0, 2, 3
    };

  // ---------------------------------------------------
  // sphere data
  int mVertexCount = 0;
  int SPHERE_VERTICES_CNT;
  int SPHERE_INDICES_CNT;
  float *SPHERE_VERTICES = nullptr;
  unsigned int *SPHERE_INDICES = nullptr;
};