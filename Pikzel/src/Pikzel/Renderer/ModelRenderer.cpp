#include "ModelRenderer.h"
#include "RenderCore.h"

#include <glm/gtx/transform.hpp>

namespace Pikzel {

   ModelRenderer::ModelRenderer(GraphicsContext& gc, std::shared_ptr<Model> model)
   : m_Model {std::move(model)}
   , m_MeshRenderer {MeshRenderer::Create(gc, *m_Model)}
   {
      CreateUniformBuffers();
      CreateFramebuffers();
      CreatePipelines(gc);
   }


   void ModelRenderer::Draw(GraphicsContext& gc, const DrawData& drawData, const glm::mat4& transform) {
      static float lightRadius = 1000.0f;
      static glm::mat4 lightProjection = glm::perspective(glm::radians(90.0f), 1.0f, 1.0f, lightRadius);

      Matrices matrices;
      matrices.viewProjection = drawData.Projection * drawData.View;
      matrices.lightSpace = drawData.LightSpace;
      matrices.eyePosition = drawData.EyePosition;
      glm::mat4 modelInvTrans = glm::mat4(glm::transpose(glm::inverse(glm::mat3(transform))));
      m_BufferMatrices->CopyFromHost(0, sizeof(Matrices), &matrices);
      m_BufferDirectionalLights->CopyFromHost(0, sizeof(DirectionalLight) * drawData.DirectionalLights.size(), drawData.DirectionalLights.data());
      m_BufferPointLights->CopyFromHost(0, sizeof(PointLight) * drawData.PointLights.size(), drawData.PointLights.data());

      // render to directional light shadow map
      {
         Pikzel::GraphicsContext& gcDirShadows = m_FramebufferDirShadow->GetGraphicsContext();
         gcDirShadows.BeginFrame();
         gcDirShadows.Bind(*m_PipelineDirShadow);
         gcDirShadows.PushConstant("constants.mvp"_hs, drawData.LightSpace * transform);

         for (const auto& mesh : m_Model->Meshes) {
            gcDirShadows.DrawIndexed(*mesh.VertexBuffer, *mesh.IndexBuffer);
         }

         gcDirShadows.EndFrame();
         gcDirShadows.SwapBuffers();
      }

      // render to point light shadow map
      {
         Pikzel::GraphicsContext& gcPtShadows = m_FramebufferPtShadow->GetGraphicsContext();

         for (int i = 0; i < drawData.PointLights.size(); ++i) {
            auto& light = drawData.PointLights[i];

            std::array<glm::mat4, 6> lightViews = {
               lightProjection * glm::lookAt(light.Position, light.Position + glm::vec3 {1.0f,  0.0f,  0.0f}, glm::vec3 {0.0f, -1.0f,  0.0f}),
               lightProjection * glm::lookAt(light.Position, light.Position + glm::vec3 {-1.0f,  0.0f,  0.0f}, glm::vec3 {0.0f, -1.0f,  0.0f}),
               lightProjection * glm::lookAt(light.Position, light.Position + glm::vec3 {0.0f,  1.0f,  0.0f}, glm::vec3 {0.0f,  0.0f,  1.0f}),
               lightProjection * glm::lookAt(light.Position, light.Position + glm::vec3 {0.0f, -1.0f,  0.0f}, glm::vec3 {0.0f,  0.0f, -1.0f}),
               lightProjection * glm::lookAt(light.Position, light.Position + glm::vec3 {0.0f,  0.0f,  1.0f}, glm::vec3 {0.0f, -1.0f,  0.0f}),
               lightProjection * glm::lookAt(light.Position, light.Position + glm::vec3 {0.0f,  0.0f, -1.0f}, glm::vec3 {0.0f, -1.0f,  0.0f}),
            };
            m_BufferLightViews->CopyFromHost(0, sizeof(glm::mat4) * lightViews.size(), lightViews.data());

            gcPtShadows.BeginFrame(i == 0 ? Pikzel::BeginFrameOp::ClearAll : Pikzel::BeginFrameOp::ClearNone);
            gcPtShadows.Bind(*m_PipelinePtShadow);
            gcPtShadows.PushConstant("constants.lightIndex"_hs, i);
            gcPtShadows.PushConstant("constants.lightRadius"_hs, lightRadius);
            gcPtShadows.Bind(*m_BufferLightViews, "UBOLightViews"_hs);
            gcPtShadows.Bind(*m_BufferPointLights, "UBOPointLights"_hs);
            gcPtShadows.PushConstant("constants.model"_hs, transform);
 
            for (const auto& mesh : m_Model->Meshes) {
               gcPtShadows.DrawIndexed(*mesh.VertexBuffer, *mesh.IndexBuffer);
            }

            gcPtShadows.EndFrame();
            gcPtShadows.SwapBuffers();
         }
      }
      gc.BeginFrame();
      m_MeshRenderer->BindToGC(gc);

      gc.PushConstant("constants.lightRadius"_hs, lightRadius);
      gc.PushConstant("constants.numPointLights"_hs, static_cast<uint32_t>(drawData.PointLights.size()));
      gc.Bind(*m_BufferMatrices, "UBOMatrices"_hs);
      gc.Bind(*m_BufferDirectionalLights, "UBODirectionalLight"_hs);
      gc.Bind(*m_BufferPointLights, "UBOPointLights"_hs);
      gc.Bind(m_FramebufferDirShadow->GetDepthTexture(), "dirShadowMap"_hs);
      gc.Bind(m_FramebufferPtShadow->GetDepthTexture(), "ptShadowMap"_hs);

      gc.PushConstant("constants.model"_hs, transform);
      for (const auto& mesh : m_Model->Meshes) {
         m_MeshRenderer->Draw(gc, mesh);
      }
   }


   void ModelRenderer::CreateUniformBuffers() {
      m_BufferMatrices = Pikzel::RenderCore::CreateUniformBuffer(sizeof(Matrices));
      m_BufferLightViews = Pikzel::RenderCore::CreateUniformBuffer(sizeof(glm::mat4) * 4 * 6);
      m_BufferDirectionalLights = RenderCore::CreateUniformBuffer(sizeof(Pikzel::DirectionalLight));
      m_BufferPointLights = RenderCore::CreateUniformBuffer(sizeof(Pikzel::PointLight) * 4);
   }


   void ModelRenderer::CreateFramebuffers() {
      m_FramebufferDirShadow = Pikzel::RenderCore::CreateFramebuffer({
         .Width = RenderCore::ShadowMapWidth,
         .Height = RenderCore::ShadowMapHeight,
         .Attachments = {{Pikzel::AttachmentType::Depth, Pikzel::TextureFormat::D32F}}
      });
      m_FramebufferPtShadow = Pikzel::RenderCore::CreateFramebuffer({
         .Width = RenderCore::ShadowMapWidth,
         .Height = RenderCore::ShadowMapHeight,
         .Layers = 4,
         .Attachments = {{Pikzel::AttachmentType::Depth, Pikzel::TextureFormat::D32F, Pikzel::TextureType::TextureCubeArray}}
      });
   }


   void ModelRenderer::CreatePipelines(GraphicsContext& gc) {
      BufferLayout layout {
         { "inPos",    Pikzel::DataType::Vec3 },
         { "inNormal", Pikzel::DataType::Vec3 },
         { "inUV",     Pikzel::DataType::Vec2 },
      };

      m_PipelineDirShadow = m_FramebufferDirShadow->GetGraphicsContext().CreatePipeline({
         layout,
         {
            { Pikzel::ShaderType::Vertex, "Scene/Shaders/Depth.vert.spv" },
            { Pikzel::ShaderType::Fragment, "Scene/Shaders/Depth.frag.spv" }
         }
      });
      m_PipelinePtShadow = m_FramebufferPtShadow->GetGraphicsContext().CreatePipeline({
         layout,
         {
            { Pikzel::ShaderType::Vertex, "Scene/Shaders/DepthCube.vert.spv" },
            { Pikzel::ShaderType::Geometry, "Scene/Shaders/DepthCube.geom.spv" },
            { Pikzel::ShaderType::Fragment, "Scene/Shaders/DepthCube.frag.spv" }
         }
      });
   }

}
