#pragma once

#include "Pikzel/Renderer/Buffer.h"
#include "Pikzel/Renderer/Texture.h"

#include <glm/glm.hpp>
#include <memory>

namespace ModelAndMeshDemo {

   // Eventually, the Pikzel engine will have its own mesh component.
   // In the meantime, this demo uses this definition of mesh
   struct Mesh {

      struct Vertex {
         Vertex(glm::vec3 pos, glm::vec3 normal, glm::vec2 uv) : Pos{ pos }, Normal{ normal }, UV{ uv } {}
         glm::vec3 Pos;
         glm::vec3 Normal;
         glm::vec2 UV;
      };

      std::pair<glm::vec3, glm::vec3> AABB = { glm::vec3{FLT_MAX}, glm::vec3{-FLT_MAX} };
      std::shared_ptr<Pikzel::VertexBuffer> VertexBuffer;
      std::shared_ptr<Pikzel::IndexBuffer> IndexBuffer;
      std::shared_ptr<Pikzel::Texture> DiffuseTexture;
      std::shared_ptr<Pikzel::Texture> SpecularTexture;
      uint32_t Index;
   };

}
