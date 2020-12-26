#include "MeshRenderer.h"

#include "Pikzel/Renderer/RenderCore.h"

namespace Pikzel {

   MeshRenderer::MeshRenderer(GraphicsContext& gc) {
      BufferLayout layout {
         { "inPos",     Pikzel::DataType::Vec3 },
         { "inNormal",  Pikzel::DataType::Vec3 },
         { "inTangent", Pikzel::DataType::Vec3 },
         { "inUV",      Pikzel::DataType::Vec2 },
      };

      m_Pipeline = gc.CreatePipeline({
         layout,
         {
            { Pikzel::ShaderType::Vertex, "Scene/Shaders/LitModel.vert.spv" },
            { Pikzel::ShaderType::Fragment, "Scene/Shaders/LitModel.frag.spv" }
         }
      });

   }


   void MeshRenderer::BindToGC(GraphicsContext& gc) const {
      gc.Bind(*m_Pipeline);
   }


   void MeshRenderer::Draw(GraphicsContext& gc, const Mesh& mesh) const {
      gc.Bind(*mesh.DiffuseTexture, "diffuseMap"_hs);
      gc.Bind(*mesh.SpecularTexture, "specularMap"_hs);
      gc.Bind(*mesh.NormalTexture, "normalMap"_hs);
      gc.DrawIndexed(*mesh.VertexBuffer, *mesh.IndexBuffer);
   }


   std::unique_ptr<MeshRenderer> MeshRenderer::Create(GraphicsContext& gc, const Model& model) {
      PKZL_CORE_ASSERT(CreateMeshRenderer, "CreateMeshRenderer procedure has not been set!");
      std::unique_ptr<MeshRenderer> meshRenderer;
      meshRenderer.reset(CreateMeshRenderer(&gc, &model));
      return meshRenderer;
   }

}
