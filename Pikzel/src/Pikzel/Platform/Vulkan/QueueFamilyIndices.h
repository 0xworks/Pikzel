#pragma once

#include <optional>

namespace Pikzel {

   struct QueueFamilyIndices {
      std::optional<uint32_t> GraphicsFamily;
      std::optional<uint32_t> PresentFamily;
      std::optional<uint32_t> ComputeFamily;
      std::optional<uint32_t> TransferFamily;
   };

}
