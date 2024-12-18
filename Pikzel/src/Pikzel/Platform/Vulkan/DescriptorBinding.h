#pragma once

#include <vulkan/vulkan.hpp>

namespace Pikzel {

   struct DescriptorBinding {
      uint32_t Binding;            // Slot to which the descriptor will be bound, corresponding to the layout index in the shader.
      uint32_t DescriptorCount;    // Number of descriptors to bind
      vk::DescriptorType Type;
      vk::ShaderStageFlags Stage;  // Shader stage at which the bound resources will be available
   };

}
