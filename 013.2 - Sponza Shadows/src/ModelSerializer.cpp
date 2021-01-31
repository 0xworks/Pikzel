#include "ModelSerializer.h"

#include "Pikzel/Renderer/RenderCore.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>


namespace SponzaShadows {

   namespace ModelSerializer {

      std::unordered_map<std::string, std::shared_ptr<Pikzel::Texture>> g_TextureCache;

      const uint32_t g_AssimpProcessFlags =
         aiProcess_Triangulate |
         aiProcess_JoinIdenticalVertices |
         aiProcess_GenNormals |
         aiProcess_GenUVCoords |
         aiProcess_CalcTangentSpace |
         aiProcess_OptimizeMeshes |
         aiProcess_ValidateDataStructure
      ;


      std::shared_ptr<Pikzel::Texture> LoadMaterialTexture(aiMaterial* mat, aiTextureType type, const std::filesystem::path& modelDir) {

         // for now we support only 0 or 1 texture  TODO: make better
         for (unsigned int i = 0; i < mat->GetTextureCount(type); ++i) {
            aiString str;
            mat->GetTexture(type, i, &str);
            std::filesystem::path texturePath = modelDir / str.C_Str();
            if (g_TextureCache.find(texturePath.string()) == g_TextureCache.end()) {

               // For now we are only loading DIFFUSE and SPECULAR texture types.
               // Make an assumption that DIFFUSE is non-linear, and SPECULAR is linear
               // TODO: when we come to needing other texture types (eg. Normals) then this needs to be revisited
               bool isSRGB = (type == aiTextureType_DIFFUSE);
               g_TextureCache[texturePath.string()] = Pikzel::RenderCore::CreateTexture({.Path = texturePath, .Format = (type == aiTextureType_DIFFUSE ? Pikzel::TextureFormat::SRGBA8 : Pikzel::TextureFormat::RGBA8)});
            }
            return g_TextureCache[texturePath.string()];
         }

         // material did not have a texture of specified type => create one
         if (type == aiTextureType_SPECULAR) {
            if (g_TextureCache.find("**black**") == g_TextureCache.end()) {
               std::shared_ptr<Pikzel::Texture> blackTexture = Pikzel::RenderCore::CreateTexture();
               uint32_t black = 0;
               blackTexture->SetData(&black, sizeof(uint32_t));
               g_TextureCache["**black**"] = blackTexture;
            }
            return g_TextureCache["**black**"];
         } else if (type == aiTextureType_NORMALS) {
            if (g_TextureCache.find("**unit-z**") == g_TextureCache.end()) {
               std::shared_ptr<Pikzel::Texture> unitVectorZ = Pikzel::RenderCore::CreateTexture();
               uint32_t normal = 0x8080ff00;
               unitVectorZ->SetData(&normal, sizeof(uint32_t));
               g_TextureCache["**unit-z**"] = unitVectorZ;
            }
            return g_TextureCache["**unit-z**"];
         }
         if (g_TextureCache.find("**white**") == g_TextureCache.end()) {
            std::shared_ptr<Pikzel::Texture> whiteTexture = Pikzel::RenderCore::CreateTexture();
            uint32_t white = 0xffffffff;
            whiteTexture->SetData(&white, sizeof(uint32_t));
            g_TextureCache["**white**"] = whiteTexture;
         }
         return g_TextureCache["**white**"];
      }


      Mesh ProcessMesh(aiMesh* pmesh, const aiScene* pscene, const std::filesystem::path& modelDir) {
         std::vector<Mesh::Vertex> vertices;
         std::vector<uint32_t> indices;
         std::vector<Pikzel::Texture> textures;

         glm::vec3 aabbMin = glm::vec3 {FLT_MAX};
         glm::vec3 aabbMax = glm::vec3 {-FLT_MAX};

         vertices.reserve(pmesh->mNumVertices);
         for (unsigned int i = 0; i < pmesh->mNumVertices; ++i) {
            vertices.emplace_back(
               glm::vec3 {pmesh->mVertices[i].x, pmesh->mVertices[i].y, pmesh->mVertices[i].z},
               glm::vec3 {pmesh->mNormals[i].x,  pmesh->mNormals[i].y,  pmesh->mNormals[i].z},
               glm::vec3 {pmesh->mTangents[i].x, pmesh->mTangents[i].y, pmesh->mTangents[i].z},
               pmesh->mTextureCoords[0] ? glm::vec2 {pmesh->mTextureCoords[0][i].x, pmesh->mTextureCoords[0][i].y} : glm::vec2 {}
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

         if (pmesh->mMaterialIndex >= 0) {
            aiMaterial* material = pscene->mMaterials[pmesh->mMaterialIndex];
            mesh.DiffuseTexture = LoadMaterialTexture(material, aiTextureType_DIFFUSE, modelDir);
            mesh.SpecularTexture = LoadMaterialTexture(material, aiTextureType_SPECULAR, modelDir);
            mesh.NormalTexture = LoadMaterialTexture(material, aiTextureType_NORMALS, modelDir);
         }

         mesh.AABB = {aabbMin, aabbMax};

         return mesh;
      }


      void ProcessNode(Model& model, aiNode* node, const aiScene* scene, const std::filesystem::path& modelDir) {
         for (unsigned int i = 0; i < node->mNumMeshes; ++i) {
            aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
            model.Meshes.emplace_back(ProcessMesh(mesh, scene, modelDir));
            model.Meshes.back().Index = model.Meshes.size() - 1;
            model.AABB = {glm::min(model.AABB.first, model.Meshes.back().AABB.first), glm::max(model.AABB.second, model.Meshes.back().AABB.second)};
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
            throw std::runtime_error {fmt::format("Error when importing model '{0}': {1}", path.string(), importer.GetErrorString())};
         }

         std::filesystem::path modelDir = path;
         modelDir.remove_filename();
         ProcessNode(*model, scene->mRootNode, scene, modelDir);

         return model;
      }


      void ClearTextureCache() {
         g_TextureCache.clear();
      }

   }
}
