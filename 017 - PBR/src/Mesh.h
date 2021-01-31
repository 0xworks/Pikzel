#pragma once

#include "Pikzel/Renderer/Buffer.h"
#include <glm/glm.hpp>
#include <memory>

namespace PBRdemo {

   struct Mesh {

      struct Vertex {
         Vertex(glm::vec3 pos, glm::vec3 normal, glm::vec3 tangent, glm::vec2 uv) : Pos{ pos }, Normal{ normal }, Tangent{ tangent }, UV{ uv } {}
         glm::vec3 Pos;
         glm::vec3 Normal;
         glm::vec3 Tangent;
         glm::vec2 UV;
      };

      std::pair<glm::vec3, glm::vec3> AABB = { glm::vec3{FLT_MAX}, glm::vec3{-FLT_MAX} };
      std::shared_ptr<Pikzel::VertexBuffer> VertexBuffer;
      std::shared_ptr<Pikzel::IndexBuffer> IndexBuffer;
      uint32_t Index;
   };

}
