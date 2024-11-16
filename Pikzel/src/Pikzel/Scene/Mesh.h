#pragma once

#include "Pikzel/Renderer/Buffer.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace Pikzel {

   struct PKZL_API Mesh final {
      PKZL_NO_COPY(Mesh);

      struct Vertex {
         Vertex(glm::vec3 pos, glm::vec3 normal, glm::vec3 tangent, glm::vec2 uv) : Pos{ pos }, Normal{ normal }, Tangent{ tangent }, UV{ uv } {}
         glm::vec3 Pos;
         glm::vec3 Normal;
         glm::vec3 Tangent;
         glm::vec2 UV;
      };

      inline static BufferLayout VertexBufferLayout = {
         { "inPos",     Pikzel::DataType::Vec3 },
         { "inNormal",  Pikzel::DataType::Vec3 },
         { "inTangent", Pikzel::DataType::Vec3 },
         { "inUV",      Pikzel::DataType::Vec2 },
      };

      Mesh() = default;
      ~Mesh() = default;

      Mesh(std::unique_ptr<VertexBuffer> vb, std::unique_ptr<IndexBuffer> ib)
      : vertexBuffer { std::move(vb) }
      , indexBuffer { std::move(ib) }
      {}

      Mesh(Mesh&& mesh) noexcept
      : vertexBuffer { std::move(mesh.vertexBuffer) }
      , indexBuffer { std::move(mesh.indexBuffer) }
      {}

      Mesh& operator=(Mesh&& mesh) noexcept {
         if (this != &mesh) {
            vertexBuffer = std::move(mesh.vertexBuffer);
            indexBuffer = std::move(mesh.indexBuffer);
         }
         return *this;
      }

      std::unique_ptr<VertexBuffer> vertexBuffer;
      std::unique_ptr<IndexBuffer> indexBuffer;
   };

}
