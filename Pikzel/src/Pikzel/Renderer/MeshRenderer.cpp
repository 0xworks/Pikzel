#include "MeshRenderer.h"

#include "Pikzel/Renderer/RenderCore.h"

namespace Pikzel {

   MeshRenderer::MeshRenderer(GraphicsContext& gc, const Model&) {

      BufferLayout layout {
         { "inPos",    Pikzel::DataType::Vec3 },
         { "inNormal", Pikzel::DataType::Vec3 },
         { "inUV",     Pikzel::DataType::Vec2 },
      };

      m_Pipeline = gc.CreatePipeline({
         layout,
         {
            { Pikzel::ShaderType::Vertex, "Scene/Shaders/Mesh.vert.spv" },
            { Pikzel::ShaderType::Fragment, "Scene/Shaders/Mesh.frag.spv" }
         }
      });

      m_DirectionalLightBuffer = RenderCore::CreateUniformBuffer(sizeof(Pikzel::DirectionalLight));
      m_PointLightBuffer = RenderCore::CreateUniformBuffer(sizeof(Pikzel::PointLight) * 4);
   }


   void MeshRenderer::BindToGC(GraphicsContext& gc) const {
      gc.Bind(*m_Pipeline);
   }


   void MeshRenderer::SetTransforms(GraphicsContext& gc, const DrawData& drawData, const glm::mat4& transform) const {
      gc.PushConstant("constants.vp"_hs, drawData.Projection * drawData.View);

      glm::mat4 modelInvTrans = glm::mat4(glm::transpose(glm::inverse(glm::mat3(transform))));
      gc.PushConstant("constants.model"_hs, transform);
      gc.PushConstant("constants.modelInvTrans"_hs, modelInvTrans);
      gc.PushConstant("constants.viewPos"_hs, drawData.ViewPosition);
   }


   void MeshRenderer::SetLighting(GraphicsContext& gc, const DrawData& drawData) const {
      m_DirectionalLightBuffer->CopyFromHost(0, sizeof(DirectionalLight) * drawData.DirectionalLights.size(), drawData.DirectionalLights.data());
      m_PointLightBuffer->CopyFromHost(0, sizeof(PointLight) * drawData.PointLights.size(), drawData.PointLights.data());
      gc.Bind(*m_DirectionalLightBuffer, "UBODirectionalLight"_hs);
      gc.Bind(*m_PointLightBuffer, "UBOPointLights"_hs);
   }


   void MeshRenderer::Draw(GraphicsContext& gc, const Mesh& mesh) const {
      gc.Bind(*mesh.DiffuseTexture, "diffuseMap"_hs);
      gc.Bind(*mesh.SpecularTexture, "specularMap"_hs);
      //gc.PushConstant("constants.shininess"_hs, mesh.Shininess);
      gc.DrawIndexed(*mesh.VertexBuffer, *mesh.IndexBuffer);
   }

}
