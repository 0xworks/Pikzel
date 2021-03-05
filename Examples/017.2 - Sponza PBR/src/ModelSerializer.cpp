#include "ModelSerializer.h"

#include "Pikzel/Renderer/RenderCore.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>


namespace SponzaPBR {

   namespace ModelSerializer {

      std::unordered_map<std::string, std::shared_ptr<Pikzel::Texture>> g_TextureCache;

      const uint32_t g_AssimpProcessFlags =
         aiProcess_Triangulate |
         aiProcess_JoinIdenticalVertices |
         aiProcess_GenNormals |
         aiProcess_GenUVCoords |
         aiProcess_CalcTangentSpace |
         aiProcess_OptimizeMeshes |
         aiProcess_ValidateDataStructure |
         aiProcess_GlobalScale
      ;


      glm::mat4 AssimpMat4ToGLMMat4(const aiMatrix4x4& matrix) {
         return {
            matrix.a1, matrix.a2, matrix.a3, matrix.a4,
            matrix.b1, matrix.b2, matrix.b3, matrix.b4,
            matrix.c1, matrix.c2, matrix.c3, matrix.c4,
            matrix.d1, matrix.d2, matrix.d3, matrix.d4
         };
      }


      std::shared_ptr<Pikzel::Texture> LoadMaterialTexture(aiMaterial* mat, aiTextureType type, const std::filesystem::path& modelDir) {

         // for now we support only 0 or 1 texture  TODO: make better
          for (unsigned int i = 0; i < mat->GetTextureCount(type); ++i) {
            aiString str;
            mat->GetTexture(type, i, &str);
            std::filesystem::path texturePath = modelDir / str.C_Str();
            if (g_TextureCache.find(texturePath.string()) == g_TextureCache.end()) {
               bool isSRGB = (type == aiTextureType_DIFFUSE);
               g_TextureCache[texturePath.string()] = Pikzel::RenderCore::CreateTexture({ .path = texturePath, .format = (type == aiTextureType_DIFFUSE ? Pikzel::TextureFormat::SRGBA8 : Pikzel::TextureFormat::RGBA8) });
            }
            return g_TextureCache[texturePath.string()];
         }

         // material did not have a texture of specified type => create one
         if (type == aiTextureType_NORMALS) {
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


      Mesh ProcessMesh(aiMesh* pmesh, const aiMatrix4x4& transform, const aiScene* pscene, const std::filesystem::path& modelDir) {
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
         mesh.Transform = AssimpMat4ToGLMMat4(transform);
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

            auto materialName = material->GetName();
            PKZL_CORE_LOG_TRACE("   Material '{0}'", materialName.data);

#if 0
            for (uint32_t i = 0; i < material->mNumProperties; i++) {
               auto prop = material->mProperties[i];

               float data = *(float*)prop->mData;
               const char* semantic = "";
               switch (prop->mSemantic) {
                  case aiTextureType_NONE:
                     semantic = "(aiTextureType_NONE)";
                     break;
                  case aiTextureType_DIFFUSE:
                     semantic = "(aiTextureType_DIFFUSE)";
                     break;
                  case aiTextureType_SPECULAR:
                     semantic = "(aiTextureType_SPECULAR)";
                     break;
                  case aiTextureType_AMBIENT:
                     semantic = "(aiTextureType_AMBIENT)";
                     break;
                  case aiTextureType_EMISSIVE:
                     semantic = "(aiTextureType_EMISSIVE)";
                     break;
                  case aiTextureType_HEIGHT:
                     semantic = "(aiTextureType_HEIGHT)";
                     break;
                  case aiTextureType_NORMALS:
                     semantic = "(aiTextureType_NORMALS)";
                     break;
                  case aiTextureType_SHININESS:
                     semantic = "(aiTextureType_SHININESS)";
                     break;
                  case aiTextureType_OPACITY:
                     semantic = "(aiTextureType_OPACITY)";
                     break;
                  case aiTextureType_DISPLACEMENT:
                     semantic = "(aiTextureType_DISPLACEMENT)";
                     break;
                  case aiTextureType_LIGHTMAP:
                     semantic = "(aiTextureType_LIGHTMAP)";
                     break;
                  case aiTextureType_REFLECTION:
                     semantic = "(aiTextureType_REFLECTION)";
                     break;
                  case aiTextureType_UNKNOWN:
                     semantic = "(aiTextureType_UNKNOWN)";
                     break;
               }
               PKZL_CORE_LOG_TRACE("      {0} = {1} {2}", prop->mKey.data, data, semantic);
            }
#endif

            mesh.AlbedoTexture = LoadMaterialTexture(material, aiTextureType_DIFFUSE, modelDir);
            mesh.MetallicRoughnessTexture = LoadMaterialTexture(material, aiTextureType_UNKNOWN, modelDir);          // HACK:  load metallic-roughness from the UNKNOWN texture type (because that's where it seems to be in the sponza model data)
            mesh.AmbientOcclusionTexture = LoadMaterialTexture(material, aiTextureType_AMBIENT_OCCLUSION, modelDir); // There is no AO texture in the sponza model data, this will just create a default one
            mesh.NormalTexture = LoadMaterialTexture(material, aiTextureType_NORMALS, modelDir);
            mesh.HeightTexture = LoadMaterialTexture(material, aiTextureType_HEIGHT, modelDir);                      // There is no height map in the sponza model data, this will just create a default one
         }

         mesh.AABB = { aabbMin, aabbMax };

         return mesh;
      }


      void ProcessNode(Model& model, aiMatrix4x4 transform, aiNode* node, const aiScene* scene, const std::filesystem::path& modelDir) {
         transform *= node->mTransformation;
         for (unsigned int i = 0; i < node->mNumMeshes; ++i) {
            aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
            model.Meshes.emplace_back(ProcessMesh(mesh, transform, scene, modelDir));
            model.Meshes.back().Index = model.Meshes.size() - 1;
            model.AABB = { glm::min(model.AABB.first, model.Meshes.back().AABB.first), glm::max(model.AABB.second, model.Meshes.back().AABB.second) };
         }
         for (unsigned int i = 0; i < node->mNumChildren; ++i) {
            ProcessNode(model, transform, node->mChildren[i], scene, modelDir);
         }
      }


      std::unique_ptr<Model> Import(const std::filesystem::path& path) {
         std::unique_ptr model = std::make_unique<Model>();

         Assimp::Importer importer;
         const aiScene* scene = importer.ReadFile(path.string(), g_AssimpProcessFlags);

         if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
            throw std::runtime_error{ fmt::format("Error when importing model '{0}': {1}", path.string(), importer.GetErrorString()) };
         }

         int upAxis = 1;
         scene->mMetaData->Get<int>("UpAxis", upAxis);
         int upAxisSign = -1;
         scene->mMetaData->Get<int>("UpAxisSign", upAxisSign);

         int frontAxis = 2;
         scene->mMetaData->Get<int>("FrontAxis", frontAxis);
         int frontAxisSign = -1;
         scene->mMetaData->Get<int>("FrontAxisSign", frontAxisSign);

         int coordAxis = 0;
         scene->mMetaData->Get<int>("CoordAxis", coordAxis);
         int coordAxisSign = 1;
         scene->mMetaData->Get<int>("CoordAxisSign", coordAxisSign);

         aiVector3D upVec = upAxis == 0 ? aiVector3D(upAxisSign, 0, 0) : upAxis == 1 ? aiVector3D(0, -upAxisSign, 0) : aiVector3D(0, 0, -upAxisSign);
         aiVector3D forwardVec = frontAxis == 0 ? aiVector3D(frontAxisSign, 0, 0) : frontAxis == 1 ? aiVector3D(0, -frontAxisSign, 0) : aiVector3D(0, 0, -frontAxisSign);
         aiVector3D rightVec = coordAxis == 0 ? aiVector3D(coordAxisSign, 0, 0) : coordAxis == 1 ? aiVector3D(0, -coordAxisSign, 0) : aiVector3D(0, 0, -coordAxisSign);
         aiMatrix4x4 mat = {
            rightVec.x,   rightVec.y,   rightVec.z,   0.0f,
            upVec.x,      upVec.y,      upVec.z,      0.0f,
            forwardVec.x, forwardVec.y, forwardVec.z, 0.0f,
            0.0f,         0.0f,         0.0f,         1.0f
         };

         std::filesystem::path modelDir = path;
         modelDir.remove_filename();
         ProcessNode(*model, mat , scene->mRootNode, scene, modelDir);

         return model;
      }


      void ClearTextureCache() {
         g_TextureCache.clear();
      }
   }

}
