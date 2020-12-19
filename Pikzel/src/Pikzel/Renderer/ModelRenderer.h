#pragma once

#include "Framebuffer.h"
#include "GraphicsContext.h"
#include "MeshRenderer.h"
#include "Pipeline.h"
#include "Pikzel/Scene/Model.h"

#include <memory>

namespace Pikzel {

   struct PKZL_API DrawData {
      const glm::mat4 Projection;
      const glm::mat4 View;
      const glm::vec3 EyePosition;
      const glm::mat4 LightSpace;
      const std::vector<DirectionalLight> DirectionalLights;
      const std::vector<PointLight> PointLights;
   };

   class PKZL_API ModelRenderer {
   public:
      ModelRenderer(GraphicsContext& gc, std::shared_ptr<Model> model);
      virtual ~ModelRenderer() = default;

      void Draw(GraphicsContext& gc, const DrawData& drawData, const glm::mat4& transform);

   private:
      void CreateUniformBuffers();
      void CreateFramebuffers();
      void CreatePipelines(GraphicsContext& gc);

   private:
      struct Matrices {
         alignas(16) glm::mat4 viewProjection;
         alignas(16) glm::mat4 lightSpace;
         alignas(16) glm::vec3 eyePosition;
      };

      std::unique_ptr<UniformBuffer> m_BufferMatrices;
      std::unique_ptr<UniformBuffer> m_BufferLightViews;
      std::unique_ptr<UniformBuffer> m_BufferDirectionalLights;
      std::unique_ptr<UniformBuffer> m_BufferPointLights;
      std::unique_ptr<Framebuffer> m_FramebufferDirShadow;
      std::unique_ptr<Framebuffer> m_FramebufferPtShadow;
      std::unique_ptr<Pipeline> m_PipelineDirShadow;
      std::unique_ptr<Pipeline> m_PipelinePtShadow;
      std::shared_ptr<Model> m_Model;
      std::unique_ptr<MeshRenderer> m_MeshRenderer;
   };

}
