#include "Pikzel/Renderer/MeshRenderer.h"

namespace Pikzel {

   std::unique_ptr<MeshRenderer> MeshRenderer::Create(GraphicsContext& gc, const Model& model) {
      return std::make_unique<MeshRenderer>(gc, model);
   }

}
