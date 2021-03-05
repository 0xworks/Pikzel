#pragma once

#include "Pikzel/Renderer/Buffer.h"
#include "Pikzel/Renderer/Texture.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <memory>

namespace SponzaPBR {

   // Eventually, the Pikzel engine will have its own mesh component.
   // In the meantime, this demo uses this definition of mesh
   struct Mesh {

      struct Vertex {
         Vertex(glm::vec3 pos, glm::vec3 normal, glm::vec3 tangent, glm::vec2 uv) : Pos{ pos }, Normal{ normal }, Tangent{ tangent }, UV{ uv } {}
         glm::vec3 Pos;
         glm::vec3 Normal;
         glm::vec3 Tangent;
         glm::vec2 UV;
      };

      glm::mat4 Transform = glm::identity<glm::mat4>();
      std::pair<glm::vec3, glm::vec3> AABB = { glm::vec3{FLT_MAX}, glm::vec3{-FLT_MAX} };
      std::shared_ptr<Pikzel::VertexBuffer> VertexBuffer;
      std::shared_ptr<Pikzel::IndexBuffer> IndexBuffer;
      std::shared_ptr<Pikzel::Texture> AlbedoTexture;
      std::shared_ptr<Pikzel::Texture> MetallicRoughnessTexture;
      std::shared_ptr<Pikzel::Texture> AmbientOcclusionTexture;
      std::shared_ptr<Pikzel::Texture> NormalTexture;
      std::shared_ptr<Pikzel::Texture> HeightTexture;
      uint32_t Index;
   };

}
