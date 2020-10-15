#include "Mesh.h"

#include "Pikzel/Renderer/RenderCore.h"

namespace Pikzel {

   MeshRenderer::MeshRenderer(GraphicsContext& gc) {

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

      // note: shader expects exactly 1
      Pikzel::DirectionalLight directionalLights[] = {
         {
            .Direction = {-0.2f, -1.0f, -0.3f},
            .Color = {0.0f, 0.0f, 0.0f},
            .Ambient = {0.01f, 0.01f, 0.01f}
         }
      };

      m_DirectionalLightBuffer = RenderCore::CreateUniformBuffer(sizeof(Pikzel::DirectionalLight));
      m_PointLightBuffer = RenderCore::CreateUniformBuffer(sizeof(Pikzel::PointLight) * 4);
   }


   MeshRenderer::~MeshRenderer() {
      ;
   }


   void MeshRenderer::BindToGC(GraphicsContext& gc) {
      gc.Bind(*m_Pipeline);
   }


   void MeshRenderer::SetTransforms(GraphicsContext& gc, const glm::mat4& projView, const glm::vec3& viewPos, const glm::mat4& transform) {
      gc.PushConstant("constants.vp"_hs, projView);

      glm::mat4 modelInvTrans = glm::mat4(glm::transpose(glm::inverse(glm::mat3(transform))));
      gc.PushConstant("constants.model"_hs, transform);
      gc.PushConstant("constants.modelInvTrans"_hs, modelInvTrans);
      gc.PushConstant("constants.viewPos"_hs, viewPos);
   }


   void MeshRenderer::SetLighting(GraphicsContext& gc, const std::vector<DirectionalLight>& directionalLights, const std::vector<PointLight>& pointLights) {
      m_DirectionalLightBuffer->CopyFromHost(0, sizeof(DirectionalLight) * directionalLights.size(), directionalLights.data());
      m_PointLightBuffer->CopyFromHost(0, sizeof(PointLight) * pointLights.size(), pointLights.data());
      gc.Bind(*m_DirectionalLightBuffer, "UBODirectionalLight"_hs);
      gc.Bind(*m_PointLightBuffer, "UBOPointLights"_hs);
   }


   void MeshRenderer::Draw(GraphicsContext& gc, const Mesh& mesh) {
      gc.Bind(*mesh.DiffuseTexture, "diffuseMap"_hs);
      gc.Bind(*mesh.SpecularTexture, "specularMap"_hs);
      //gc.PushConstant("constants.shininess"_hs, mesh.Shininess);
      gc.DrawIndexed(*mesh.VertexBuffer, *mesh.IndexBuffer);
   }

}
