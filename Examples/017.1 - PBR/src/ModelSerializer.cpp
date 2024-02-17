#include "ModelSerializer.h"

#include "Pikzel/Renderer/RenderCore.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <filesystem>
#include <format>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>


namespace PBRdemo {

   namespace ModelSerializer {

      const uint32_t g_AssimpProcessFlags =
         aiProcess_Triangulate |
         aiProcess_JoinIdenticalVertices |
         aiProcess_GenNormals |
         aiProcess_GenUVCoords |
         aiProcess_CalcTangentSpace |
         aiProcess_OptimizeMeshes |
         aiProcess_ValidateDataStructure
      ;


      Mesh ProcessMesh(aiMesh* pmesh, const aiScene* pscene, const std::filesystem::path& modelDir) {
         std::vector<Mesh::Vertex> vertices;
         std::vector<uint32_t> indices;

         glm::vec3 aabbMin = glm::vec3{ FLT_MAX };
         glm::vec3 aabbMax = glm::vec3{ -FLT_MAX };

         vertices.reserve(pmesh->mNumVertices);
         for (unsigned int i = 0; i < pmesh->mNumVertices; ++i) {
            vertices.emplace_back(
               glm::vec3{ pmesh->mVertices[i].x, pmesh->mVertices[i].y, pmesh->mVertices[i].z },
               glm::vec3{ pmesh->mNormals[i].x,  pmesh->mNormals[i].y,  pmesh->mNormals[i].z },
               glm::vec3{ pmesh->mTangents[i].x, pmesh->mTangents[i].y, pmesh->mTangents[i].z },
               pmesh->mTextureCoords[0] ? glm::vec2{ pmesh->mTextureCoords[0][i].x, pmesh->mTextureCoords[0][i].y } : glm::vec2{}
            );
            aabbMin = glm::min(aabbMin, vertices.back().Pos);
            aabbMax = glm::max(aabbMax, vertices.back().Pos);
         }

         indices.reserve(pmesh->mNumFaces * 3);
         for (unsigned int i = 0; i < pmesh->mNumFaces; ++i) {
            aiFace face = pmesh->mFaces[i];
            for (unsigned int j = 0; j < face.mNumIndices; ++j) {
               indices.emplace_back(face.mIndices[j]);
            }
         }

         Mesh mesh;
         mesh.VertexBuffer = Pikzel::RenderCore::CreateVertexBuffer(
            {
               { "inPos",     Pikzel::DataType::Vec3 },
               { "inNormal",  Pikzel::DataType::Vec3 },
               { "inTangent", Pikzel::DataType::Vec3 },
               { "inUV",      Pikzel::DataType::Vec2 },
            },
            vertices.size() * sizeof(Mesh::Vertex),
            vertices.data()
         );

         mesh.IndexBuffer = Pikzel::RenderCore::CreateIndexBuffer(indices.size(), indices.data());
         mesh.AABB = { aabbMin, aabbMax };

         return mesh;
      }


      void ProcessNode(Model& model, aiNode* node, const aiScene* scene, const std::filesystem::path& modelDir) {
         for (unsigned int i = 0; i < node->mNumMeshes; ++i) {
            aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
            model.Meshes.emplace_back(ProcessMesh(mesh, scene, modelDir));
            model.Meshes.back().Index = model.Meshes.size() - 1;
            model.AABB = { glm::min(model.AABB.first, model.Meshes.back().AABB.first), glm::max(model.AABB.second, model.Meshes.back().AABB.second) };
         }
         for (unsigned int i = 0; i < node->mNumChildren; ++i) {
            ProcessNode(model, node->mChildren[i], scene, modelDir);
         }
      }


      std::unique_ptr<Model> Import(const std::filesystem::path& path) {
         std::unique_ptr model = std::make_unique<Model>();

         Assimp::Importer importer;
         const aiScene* scene = importer.ReadFile(path.string(), g_AssimpProcessFlags);

         if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
            throw std::runtime_error{std::format("Error when importing model '{}': {}", path, importer.GetErrorString())};
         }

         std::filesystem::path modelDir = path;
         modelDir.remove_filename();
         ProcessNode(*model, scene->mRootNode, scene, modelDir);

         return model;
      }

   }
}
