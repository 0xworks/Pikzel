#pragma once

#include "GraphicsContext.h"
#include "MeshRenderer.h"
#include "Pikzel/Scene/Model.h"

#include <memory>

namespace Pikzel {

   class ModelRenderer {
   public:
      ModelRenderer(GraphicsContext& gc, std::unique_ptr<Model> model);
      virtual ~ModelRenderer() = default;

      void Draw(GraphicsContext& gc, const DrawData& drawData, const glm::mat4& transform);

   private:
      std::unique_ptr<Model> m_Model;
      std::unique_ptr<MeshRenderer> m_MeshRenderer;
   };

}
