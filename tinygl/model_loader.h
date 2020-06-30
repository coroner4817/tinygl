#pragma once

#include <tinygl/common.h>
#include <tinygl/texture_manager.h>
#include <tinygl/VAO_manager.h>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

// user define Vertex and Texture data structure
// one VAO per mesh
template <typename V>
class Mesh: public TextureManager, public VAOManager
{
public:
  std::string name;
  unsigned int vertices_sz;
  unsigned int VAO;
  unsigned int textureChannel_;

  Mesh(const std::string& n, unsigned int sz, unsigned int textureChannel) : name(n), vertices_sz(sz), textureChannel_(textureChannel){}
  ~Mesh() {}

  void cleanUpMesh(){
    cleanUpVAOs();
    cleanUpTextures();
  }

  unsigned int getTextureChannelID() const {
    return textureChannel_;
  }

  void generateTexture2D(aiMaterial* material, const std::vector<aiTextureType>& textureTypes, const std::string& directory){
    for(const auto& type : textureTypes){
      for(unsigned int i = 0; i < material->GetTextureCount(type); i++){
        aiString path;
        material->GetTexture(type, i, &path);
        loadTexture2D(directory + "/" + std::string(path.C_Str()), textureChannel_++, (int)type);
      }
    }
  }

  void generateVAO(const void *vertices, int vertices_sz, const std::vector<AttrProperity> &attrprop, const void *indices, int indices_sz){
    VAO = generateNewVAOFromArray(name, vertices, vertices_sz, attrprop, indices, indices_sz);
  }

  void Draw(Shader shader){
    for(const auto& ti : textureTypeActiveIDMap){
      shader.setTexturesByType(int(ti.first), ti.second);
    }
    bindAllTextures();

    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, vertices_sz, GL_UNSIGNED_INT, 0);

    // unbind after draw
    glBindVertexArray(0);
    glActiveTexture(GL_TEXTURE0);
  }
};

template <typename DataType, typename IterType>
class VAOIterator{

public:
  VAOIterator(IterType it) : it_(it){};
  bool operator==(const VAOIterator& other) const {
    return it_ == other.it_;
  }
  bool operator!=(const VAOIterator& other) const {
    return it_ != other.it_;
  }
  VAOIterator& operator++(){
    ++it_;
    return *this;
  }
  VAOIterator operator++(int){
    VAOIterator temp = *this;
    operator++();
    return temp;
  }
  DataType& operator*() const {
    return (*it_).VAO;
  }

private:
  IterType it_;
};

// user define Vertex and Texture data structure
template <typename V>
class ModelLoader
{
public:
  ModelLoader(unsigned int startTextureChannel)
  {
    textureChannel_ = startTextureChannel;
    processNodeHandler_ = std::bind(&ModelLoader::processNodeDefault, this, std::placeholders::_1, std::placeholders::_2);
  }

  ~ModelLoader(){
    for(auto& ml : models_){
      for(auto& m : ml.second)
        m.cleanUpMesh();
    }
  }

  typedef VAOIterator<unsigned int, typename std::vector<Mesh<V>>::iterator> vaoIterator;
  vaoIterator GetVAOIteratorBegin(const std::string& name){
    return vaoIterator(models_[name].begin());
  }
  vaoIterator GetVAOIteratorEnd(const std::string& name){
    return vaoIterator(models_[name].end());
  }

  void setProcessNodeHandler(const std::function<void(aiNode *, const aiScene *)>&& h)
  {
    processNodeHandler_ = h;
  }

  void setProcessMeshHandler(const std::function<Mesh<V>(aiMesh *, const aiScene *, const std::string& directory, const std::string& name, unsigned int chlSt)>&& h)
  {
    processMeshHandler_ = h;
  }

  void load(const std::string &path, const std::string &name)
  {
    if(!processNodeHandler_ || !processMeshHandler_){
      std::cout << "ERROR::MODELLOADER:: need to set process*handler before load" << std::endl;
      return;
    }

    meshTemp_.clear();
    nameTemp_ = name;
    // note that we don't clear the models because we can keep loading models and draw the mesh together
    modelPath_ = path;
    // retrieve the directory path of the filepath
    modelDir_ = modelPath_.substr(0, modelPath_.find_last_of('/'));

    // read file via ASSIMP
    Assimp::Importer importer;
    // aiProcess_CalcTangentSpace bit will trigger calculation of the tangent and bittangent so that we can use the normal map
    // the normal map is store in the HEIGHT image
    const aiScene *scene = importer.ReadFile(modelPath_, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);
    // check for errors
    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) // if is Not Zero
    {
      std::cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << std::endl;
      return;
    }
    // process ASSIMP's root node recursively
    processNodeHandler_(scene->mRootNode, scene);

    // at here we have finished all node and all mesh belong to this model has been push to meshTemp_
    models_[name] = meshTemp_;
    meshTemp_.clear();
  };

  void remove(const std::string& name)
  {
    if(models_.find(name) != models_.end()){
      for(const auto& m : models_[name]){
        m.cleanUpMesh();
      }
      models_.erase(name);
    }
  }

  void Draw(const std::vector<std::string>& names, Shader shader)
  {
    for(const auto& n : names){
      for(auto& m : models_[n]){
        m.Draw(shader);
      }
    }
  }

  // TODO: change to a static texrture ID Manager
  unsigned int getLastTextureChannelID() const {
    return textureChannel_;
  }

private:
  unsigned int textureChannel_;
  std::string modelPath_;
  std::string modelDir_;

  std::function<void(aiNode *, const aiScene *)> processNodeHandler_ = nullptr;
  std::function<Mesh<V>(aiMesh *, const aiScene *, const std::string& directory, const std::string& name, unsigned int chlSt)> processMeshHandler_ = nullptr;

  std::vector<Mesh<V>> meshTemp_;
  std::string nameTemp_;

  std::unordered_map<std::string, std::vector<Mesh<V>>> models_;

  void processNodeDefault(aiNode *node, const aiScene *scene)
  {
    // process each mesh located at the current node
    for (unsigned int i = 0; i < node->mNumMeshes; i++)
    {
      // the node object only contains indices to index the actual objects in the scene.
      // the scene contains all the data, node is just to keep stuff organized (like relations between nodes).
      aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
      meshTemp_.push_back(processMeshHandler_(mesh, scene, modelDir_, nameTemp_ + std::to_string(meshTemp_.size()), textureChannel_));
      textureChannel_ = meshTemp_.back().getTextureChannelID();
    }
    // after we've processed all of the meshes (if any) we then recursively process each of the children nodes
    for (unsigned int i = 0; i < node->mNumChildren; i++)
    {
      processNodeDefault(node->mChildren[i], scene);
    }
  }
};


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// standard model loader marco
#define STANDARD_OBJ_LOADER(VERTEX, TEXTURE)                                                                                  \
[=](aiMesh *mesh, const aiScene *scene, const std::string& directory, const std::string& name,  unsigned int chlSt) -> Mesh<VERTEX> {\
    std::vector<VERTEX> vertices;                                                                                             \
    std::vector<unsigned int> indices;                                                                                        \
    /* process vertices */                                                                                                    \
    for (unsigned int i = 0; i < mesh->mNumVertices; i++)                                                                     \
    {                                                                                                                         \
      Vertex vertex;                                                                                                          \
      /* we declare a placeholder vector since assimp uses its own vector class that  */                                      \
      /* doesn't directly convert to glm's vec3 class so we transfer the data to this placeholder glm::vec3 first. */         \
      glm::vec3 vector;                                                                                                       \
      /* positions */                                                                                                         \
      vector.x = mesh->mVertices[i].x;                                                                                        \
      vector.y = mesh->mVertices[i].y;                                                                                        \
      vector.z = mesh->mVertices[i].z;                                                                                        \
      vertex.Position = vector;                                                                                               \
      /* normals */                                                                                                           \
      vector.x = mesh->mNormals[i].x;                                                                                         \
      vector.y = mesh->mNormals[i].y;                                                                                         \
      vector.z = mesh->mNormals[i].z;                                                                                         \
      vertex.Normal = vector;                                                                                                 \
      /* texture coordinates */                                                                                               \
      if (mesh->mTextureCoords[0])                                                                                            \
      {                                                                                                                       \
        glm::vec2 vec;                                                                                                        \
        /* a vertex can contain up to 8 different texture coordinates. We thus make the assumption that we won't */           \
        /* use models where a vertex can have multiple texture coordinates so we always take the first set (0). */            \
        vec.x = mesh->mTextureCoords[0][i].x;                                                                                 \
        vec.y = mesh->mTextureCoords[0][i].y;                                                                                 \
        vertex.TexCoords = vec;                                                                                               \
      }                                                                                                                       \
      else                                                                                                                    \
        vertex.TexCoords = glm::vec2(0.0f, 0.0f);                                                                             \
      /* tangent */                                                                                                           \
      /* notice that tangents and bitangent might not exist, so the data to the shader might not valid*/                      \
      if(mesh->mTangents){                                                                                                    \
        vector.x = mesh->mTangents[i].x;                                                                                      \
        vector.y = mesh->mTangents[i].y;                                                                                      \
        vector.z = mesh->mTangents[i].z;                                                                                      \
        vertex.Tangent = vector;                                                                                              \
      }                                                                                                                       \
      /* bitangent */                                                                                                         \
      if(mesh->mBitangents){                                                                                                  \
        vector.x = mesh->mBitangents[i].x;                                                                                    \
        vector.y = mesh->mBitangents[i].y;                                                                                    \
        vector.z = mesh->mBitangents[i].z;                                                                                    \
        vertex.Bitangent = vector;                                                                                            \
      }                                                                                                                       \
      vertices.push_back(vertex);                                                                                             \
    }                                                                                                                         \
    /*now walk through each of the mesh's triangles and retrieve the corresponding vertex indices.*/                          \
    for (unsigned int i = 0; i < mesh->mNumFaces; i++)                                                                        \
    {                                                                                                                         \
      aiFace face = mesh->mFaces[i];                                                                                          \
      /* retrieve all indices of the face and store them in the indices vector */                                             \
      for (unsigned int j = 0; j < face.mNumIndices; j++)                                                                     \
        indices.push_back(face.mIndices[j]);                                                                                  \
    }                                                                                                                         \
    /* extract materials */                                                                                                   \
    aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];                                                           \
    Mesh<VERTEX> m_ret(name, indices.size(), chlSt);                                                                          \
    m_ret.generateTexture2D(material, TEXTURE, directory);                                                                    \
    m_ret.generateVAO(&vertices[0], vertices.size() * sizeof(Vertex), {{0, 3}, {1, 3}, {2, 2}, {3, 3}, {4, 3}},               \
                    &indices[0], indices.size() * sizeof(unsigned int));                                                      \
    return m_ret;                                                                                                             \
  }
