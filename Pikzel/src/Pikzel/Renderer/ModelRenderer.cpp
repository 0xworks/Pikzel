#include "ModelRenderer.h"

namespace Pikzel {

   ModelRenderer::ModelRenderer(GraphicsContext& gc, std::shared_ptr<Model> model)
   : m_Model {std::move(model)}
   , m_MeshRenderer {MeshRenderer::Create(gc, *m_Model)}
   {}


   void ModelRenderer::Draw(GraphicsContext& gc, const DrawData& drawData, const glm::mat4& transform) {
      m_MeshRenderer->BindToGC(gc);
      m_MeshRenderer->SetTransforms(gc, drawData, transform);
      m_MeshRenderer->SetLighting(gc, drawData);

      for (const auto& mesh : m_Model->Meshes) {
         m_MeshRenderer->Draw(gc, mesh);
      }
   }

}
