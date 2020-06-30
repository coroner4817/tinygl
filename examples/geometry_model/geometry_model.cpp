#include "tinygl/glApp.h"
#include "tinygl/camera.h"
#include "tinygl/shader_s.h"
#include "tinygl/model_loader.h"
#include "tinygl/default_mesh.h"

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
std::vector<aiTextureType> nanosuitTextureTypes = {aiTextureType_DIFFUSE, aiTextureType_SPECULAR, aiTextureType_HEIGHT};

int main()
{
  int lampNum = 4;
  glm::vec3 lampPosVec[] = {
    glm::vec3(2.f, 2.f, 1.f),
    glm::vec3(2.f, -2.f, -1.f),
    glm::vec3(-2.f, 2.f, 1.f),
    glm::vec3(-2.f, -2.f, -1.f)
  };
  glm::vec3 lampRotAxisVec[] = {
    glm::vec3(0, 0, 1),
    glm::vec3(0, 1, 0),
    glm::vec3(1, 0, 0),
    glm::vec3(0, -1, 0)
  };

  GLApp app;
  app.init();
  app.setProjectionMatrix(45.f, 0.1f, 100.f);
  Camera cam;
  cam.addViewMatrixUpdateListener(&app);
  DefaultMesh dm;
  dm.initSphereData(1, 15, 15);

  // maybe should let the modelloader manage the texture
  auto modelLoader = ModelLoader<Vertex>(5);
  modelLoader.setProcessMeshHandler([=, &app](aiMesh *mesh, const aiScene *scene, const std::string& directory, const std::string& name, unsigned int chlSt) -> Mesh<Vertex> {
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    // process vertices
    for (unsigned int i = 0; i < mesh->mNumVertices; i++)
    {
      Vertex vertex;
      glm::vec3 vector; // we declare a placeholder vector since assimp uses its own vector class that doesn't directly convert to glm's vec3 class so we transfer the data to this placeholder glm::vec3 first.
      // positions
      vector.x = mesh->mVertices[i].x;
      vector.y = mesh->mVertices[i].y;
      vector.z = mesh->mVertices[i].z;
      vertex.Position = vector;
      // normals
      vector.x = mesh->mNormals[i].x;
      vector.y = mesh->mNormals[i].y;
      vector.z = mesh->mNormals[i].z;
      vertex.Normal = vector;
      // texture coordinates
      if (mesh->mTextureCoords[0]) // does the mesh contain texture coordinates?
      {
        glm::vec2 vec;
        // a vertex can contain up to 8 different texture coordinates. We thus make the assumption that we won't
        // use models where a vertex can have multiple texture coordinates so we always take the first set (0).
        vec.x = mesh->mTextureCoords[0][i].x;
        vec.y = mesh->mTextureCoords[0][i].y;
        vertex.TexCoords = vec;
      }
      else
        vertex.TexCoords = glm::vec2(0.0f, 0.0f);
      // tangent
      vector.x = mesh->mTangents[i].x;
      vector.y = mesh->mTangents[i].y;
      vector.z = mesh->mTangents[i].z;
      vertex.Tangent = vector;
      // bitangent
      vector.x = mesh->mBitangents[i].x;
      vector.y = mesh->mBitangents[i].y;
      vector.z = mesh->mBitangents[i].z;
      vertex.Bitangent = vector;
      vertices.push_back(vertex);
    }

    // now walk through each of the mesh's faces (a face is a mesh its triangle) and retrieve the corresponding vertex indices.
    for (unsigned int i = 0; i < mesh->mNumFaces; i++)
    {
      aiFace face = mesh->mFaces[i];
      // retrieve all indices of the face and store them in the indices vector
      for (unsigned int j = 0; j < face.mNumIndices; j++)
        indices.push_back(face.mIndices[j]);
    }

    // extract materials
    aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

    Mesh<Vertex> m_ret(name, indices.size(), chlSt);
    m_ret.generateTexture2D(material, nanosuitTextureTypes, directory);
    m_ret.generateVAO(&vertices[0], vertices.size() * sizeof(Vertex), {{0, 3}, {1, 3}, {2, 2}, {3, 3}, {4, 3}},
                    &indices[0], indices.size() * sizeof(unsigned int));
    return m_ret;
  });

  modelLoader.load(ASSETS_3D_BASEPATH + "nanosuit/nanosuit.obj", "nanosuit");

  Shader modelShader(getExecutablePath() + "/mesh.vs", getExecutablePath() + "/mesh.fs", getExecutablePath() + "/mesh.gs");
  modelShader.init("model", "view", &cam, "projection", app.getProjectionMatrix());
  modelShader.setTextureBaseName(aiTextureType_DIFFUSE, "texture_diffuse");
  modelShader.setTextureBaseName(aiTextureType_SPECULAR, "texture_specular");
  modelShader.setTextureBaseName(aiTextureType_HEIGHT, "texture_normal");

  modelShader.setModelMatrix(glm::vec3(0.0f, -1.75f, 0.0f), glm::vec3(0.2f, 0.2f, 0.2f));
  modelShader.setFloat("texture_shinness", 256.0f);

  modelShader.setVec3("directlight.direction", {-0.2f, -1.0f, -0.3f});
  modelShader.setVec3("directlight.ambient", {0.05f, 0.05f, 0.05f});
  modelShader.setVec3("directlight.diffuse", {0.3f, 0.3f, 0.3f});
  modelShader.setVec3("directlight.specular", {0.2f, 0.2f, 0.2f});

  for(int i = 0; i < lampNum; ++i){
    modelShader.setVec3("pointlights[" + std::to_string(i) + "].ambient",  {.1f, .1f, .1f});
    modelShader.setVec3("pointlights[" + std::to_string(i) + "].diffuse",  {.5f, 0.5f, 0.5f});
    modelShader.setVec3("pointlights[" + std::to_string(i) + "].specular", {1.f, 1.f, 1.f});
    modelShader.setFloat("pointlights[" + std::to_string(i) + "].constant",  1.0f);
    modelShader.setFloat("pointlights[" + std::to_string(i) + "].linear",    0.05f);
    modelShader.setFloat("pointlights[" + std::to_string(i) + "].quadratic", 0.015f);
  }

  modelShader.setVec3("spotlight.position", cam.GetCameraPos());
  modelShader.setVec3("spotlight.direction", cam.GetCameraFront());
  modelShader.setFloat("spotlight.innerCutoff", glm::cos(glm::radians(12.5f)));
  modelShader.setFloat("spotlight.outerCutoff", glm::cos(glm::radians(17.5f)));

  Shader normalVecShader(getExecutablePath() + "/normalVec.vs", getExecutablePath() + "/normalVec.fs", getExecutablePath() + "/normalVec.gs");
  normalVecShader.init("model", "view", &cam, "projection", app.getProjectionMatrix());
  normalVecShader.setModelMatrix(glm::vec3(0.0f, -1.75f, 0.0f), glm::vec3(0.2f, 0.2f, 0.2f));

  Shader lampShader(getExecutablePath() + "/lamp.vs", getExecutablePath() + "/lamp.fs");
  lampShader.init("model", "view", &cam, "projection", app.getProjectionMatrix());
  lampShader.setModelMatrix();
  app.generateNewVAOFromArray("lampSphere", dm.getSphereVerticesArray(), dm.getSphereVerticesArraySize(), {{0, 3}}, dm.getSphereIndicesArray(), dm.getSphereIndicesArraySize());

  app.run([=, &lampShader, &modelShader, &normalVecShader, &modelLoader, &cam, &app, &dm](float a_delta){
    cam.update(app.getWindow(), a_delta);

    lampShader.use();
    app.useVAO("lampSphere");
    std::vector<glm::vec3> rotateLampPosVec;
    for(int i = 0;i<lampNum;++i){
      glm::vec3 rotateLampPos = glm::rotate(lampPosVec[i]-lampRotAxisVec[i]*lampPosVec[i], (float)glfwGetTime(), lampRotAxisVec[i]) + lampRotAxisVec[i]*lampPosVec[i];
      rotateLampPosVec.push_back(rotateLampPos);
      lampShader.setModelMatrix(rotateLampPos, glm::vec3(0.2f), (float)glfwGetTime(), glm::vec3(1, 1, 1));
      glDrawElements(GL_TRIANGLES, dm.getSphereVerticesCount(), GL_UNSIGNED_INT, 0);
    }

    modelShader.use();
    for(int i = 0; i< lampNum; ++i){
      modelShader.setVec3("pointlights[" + std::to_string(i) + "].position", rotateLampPosVec[i]);
    }
    modelShader.setVec3("cameraPos", cam.GetCameraPos());
    modelShader.setVec3("spotlight.position", cam.GetCameraPos());
    modelShader.setVec3("spotlight.direction", cam.GetCameraFront());
    modelShader.setFloat("explode_time", glfwGetTime());

    modelLoader.Draw({"nanosuit"}, modelShader);

    normalVecShader.use();
    modelLoader.Draw({"nanosuit"}, normalVecShader);
  });
}