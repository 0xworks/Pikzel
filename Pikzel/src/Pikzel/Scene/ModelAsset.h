#pragma once

#include "Mesh.h"

#include <vector>

namespace Pikzel {

   struct PKZL_API ModelAsset final {
      PKZL_NO_COPYMOVE(ModelAsset);

      ModelAsset() noexcept = default;

      std::vector<Mesh> Meshes;
   };

}
