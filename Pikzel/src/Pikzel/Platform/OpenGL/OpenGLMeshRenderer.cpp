#include "Pikzel/Renderer/MeshRenderer.h"

namespace Pikzel {

   extern "C" __declspec(dllexport) MeshRenderer* CreateMeshRenderer(GraphicsContext* gc, const Model* model) {
      PKZL_CORE_ASSERT(gc, "GraphicsContext was null in call to CreateMeshRenderer!");
      return new MeshRenderer {*gc};
   }

}
