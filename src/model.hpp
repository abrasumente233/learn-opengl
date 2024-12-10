#pragma once

#include <cstdio>
#include <string>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <glad/glad.h>

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include "mesh.hpp"
#include "shader.hpp"
#include "texture.hpp"

std::vector<Texture> textures_loaded;

class Model {
public:
  Model(const char *path) { load_model(path); }

  void draw(Shader &shader) {
    for (auto &mesh : meshes) {
      mesh.draw(shader);
    }
  }

private:
  std::vector<Mesh> meshes;
  std::string directory;

  void load_model(const std::string &path) {
    Assimp::Importer import;
    const aiScene *scene =
      import.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE ||
        !scene->mRootNode) {
      fprintf(stderr, "Failed to load model: %s\n", import.GetErrorString());
      return;
    }
    directory = path.substr(0, path.find_last_of('/'));

    process_node(scene->mRootNode, scene);
  }

  void process_node(aiNode *node, const aiScene *scene) {
    for (size_t i = 0; i < node->mNumMeshes; i++) {
      aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
      meshes.push_back(process_mesh(mesh, scene));
    }

    for (size_t i = 0; i < node->mNumChildren; i++) {
      process_node(node->mChildren[i], scene);
    }
  }

  Mesh process_mesh(aiMesh *mesh, const aiScene *scene) {
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    std::vector<Texture> textures;

    for (size_t i = 0; i < mesh->mNumVertices; i++) {
      Vertex vertex;
      glm::vec3 vector;
      vector.x = mesh->mVertices[i].x;
      vector.y = mesh->mVertices[i].y;
      vector.z = mesh->mVertices[i].z;
      vertex.position = vector;

      vector.x = mesh->mNormals[i].x;
      vector.y = mesh->mNormals[i].y;
      vector.z = mesh->mNormals[i].z;
      vertex.normal = vector;

      if (mesh->mTextureCoords[0]) {
        glm::vec2 vec;
        vec.x = mesh->mTextureCoords[0][i].x;
        vec.y = mesh->mTextureCoords[0][i].y;
        vertex.tex_coords = vec;
      } else {
        vertex.tex_coords = glm::vec2(0.0f, 0.0f);
      }

      vertices.push_back(vertex);
    }

    for (size_t i = 0; i < mesh->mNumFaces; i++) {
      aiFace face = mesh->mFaces[i];
      for (size_t j = 0; j < face.mNumIndices; j++) {
        indices.push_back(face.mIndices[j]);
      }
    }

    if (mesh->mMaterialIndex >= 0) {
      aiMaterial *material = scene->mMaterials[mesh->mMaterialIndex];
      std::vector<Texture> diffuse_maps = load_material_textures(
        material, aiTextureType_DIFFUSE, TextureType::DIFFUSE);
      textures.insert(textures.end(), diffuse_maps.begin(), diffuse_maps.end());
      std::vector<Texture> specular_maps = load_material_textures(
        material, aiTextureType_SPECULAR, TextureType::SPECULAR);
      textures.insert(textures.end(), specular_maps.begin(),
                      specular_maps.end());
    }

    return Mesh(vertices, indices, textures);
  }

  std::vector<Texture> load_material_textures(aiMaterial *mat,
                                              aiTextureType type,
                                              TextureType texture_type) {
    std::vector<Texture> textures;
    for (size_t i = 0; i < mat->GetTextureCount(type); i++) {
      aiString str;
      mat->GetTexture(type, i, &str);
      std::string path = directory + "/" + str.C_Str();
      bool skip = false;
      for (size_t j = 0; j < textures_loaded.size(); j++) {
        if (std::strcmp(textures_loaded[j].path.c_str(), path.c_str()) == 0) {
          textures.push_back(textures_loaded[j]);
          skip = true;
          break;
        }
      }
      if (!skip) {
        auto texture = Texture(path.c_str(), texture_type);
        textures.push_back(texture);
        textures_loaded.push_back(texture);
      }
    }
    return textures;
  }
};
