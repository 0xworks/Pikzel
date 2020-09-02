#pragma once

#include <vulkan/vulkan.hpp>
#include <vector>

namespace Pikzel {

   struct SwapChainSupportDetails {
      vk::SurfaceCapabilitiesKHR Capabilities;
      std::vector<vk::SurfaceFormatKHR> Formats;
      std::vector<vk::PresentModeKHR> PresentModes;
   };

}
