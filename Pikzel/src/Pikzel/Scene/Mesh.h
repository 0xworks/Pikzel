#pragma once

#include "Pikzel/Renderer/Buffer.h"
#include "Pikzel/Renderer/Texture.h"

#include <glm/glm.hpp>

#include <utility>

namespace Pikzel {

   struct Mesh {

      struct Vertex {
         Vertex(glm::vec3 pos, glm::vec3 normal, glm::vec2 uv) : Pos {pos}, Normal { normal }, UV {uv} {}
         glm::vec3 Pos;
         glm::vec3 Normal;
         glm::vec2 UV;
      };

      Mesh() = default;
      Mesh(const Mesh&) = default;
      Mesh(Mesh&&) = default;
      ~Mesh() {
         ;
      }

      std::pair<glm::vec3, glm::vec3> AABB = {glm::vec3{FLT_MAX}, glm::vec3{-FLT_MAX}};
      std::unique_ptr<Pikzel::VertexBuffer> VertexBuffer;
      std::unique_ptr<Pikzel::IndexBuffer> IndexBuffer;
      std::shared_ptr<Pikzel::Texture2D> DiffuseTexture;
      std::shared_ptr<Pikzel::Texture2D> SpecularTexture;
      uint32_t Index;
   };

}
