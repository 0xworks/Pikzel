#include "Model.h"

#include "Light.h"

namespace Pikzel {

   ModelRenderer::ModelRenderer(GraphicsContext& gc) {
      m_MeshRenderer = std::make_unique<MeshRenderer>(gc);
   }


   ModelRenderer::~ModelRenderer() {
      ;
   }

   void ModelRenderer::Draw(GraphicsContext& gc, const DrawData drawData, const Model& model, const glm::mat4& transform) {
      m_MeshRenderer->BindToGC(gc);
      m_MeshRenderer->SetTransforms(gc, drawData.ProjView, drawData.ViewPos, transform);
      m_MeshRenderer->SetLighting(gc, drawData.DirectionalLights, drawData.PointLights);

      for (auto& mesh : model.Meshes) {
         m_MeshRenderer->Draw(gc, mesh);
      }
   }

}
