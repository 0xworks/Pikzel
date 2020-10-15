#pragma once

#include "Pikzel/Renderer/Buffer.h"
#include "Pikzel/Renderer/GraphicsContext.h"
#include "Pikzel/Renderer/Texture.h"
#include "Pikzel/Scene/Light.h"

#include <glm/glm.hpp>

namespace Pikzel {

   struct Mesh {

      struct Vertex {

         Vertex(glm::vec3 pos, glm::vec3 normal, glm::vec2 uv) : Pos {pos}, Normal { normal }, UV {uv} {}
         glm::vec3 Pos;
         glm::vec3 Normal;
         glm::vec2 UV;
      };

      std::unique_ptr<Pikzel::VertexBuffer> VertexBuffer;
      std::unique_ptr<Pikzel::IndexBuffer> IndexBuffer;
      std::shared_ptr<Pikzel::Texture2D> DiffuseTexture;
      std::shared_ptr<Pikzel::Texture2D> SpecularTexture;
   };


   class MeshRenderer {
   public:
      MeshRenderer(GraphicsContext& gc);
      virtual ~MeshRenderer();

      void BindToGC(GraphicsContext& gc);

      void SetTransforms(GraphicsContext& gc, const glm::mat4& projView, const glm::vec3& viewPos, const glm::mat4& transform);
      void SetLighting(GraphicsContext& gc, const std::vector<DirectionalLight>& directionalLights, const std::vector<PointLight>& pointLights);
      
      void Draw(GraphicsContext& gc, const Mesh& mesh);

   private:
      std::unique_ptr<UniformBuffer> m_DirectionalLightBuffer;
      std::unique_ptr<UniformBuffer> m_PointLightBuffer;
      std::unique_ptr<Pipeline> m_Pipeline;
   };

}
