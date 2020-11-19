#pragma once

#include "Pikzel/Core/Core.h"

#include <vulkan/vulkan.hpp>

namespace Pikzel {

   class VulkanFence final {
      // Manages construction and desctruction of a vk::Fence so that we can
      // bung these things into a std::shared_ptr<>

      // Why on earth would we want a std::shared_ptr<Fence>?
      // Good question. I don't like it either!
      // The current use-case is to manage descriptors.
      // Descriptors are created on-demand by a pipeline state object.
      // They then get bound to a cmd buffer by a graphics context just before draw calls
      // When the cmd buffer is submitted, the graphics context hands a fence to the pipeline so that the pipeline
      // knows that it cannot overwrite the descriptor until that fence is signalled.
      // If ownership of the fence is not shared, then there is a chance that the graphics context could 
      // get destroyed (destroying the fence with it), leaving the pipeline with a dangling pointer

   public:
      VulkanFence(vk::Device device)
      : m_Device {device}
      {
         PKZL_CORE_ASSERT(device, "null device");
         m_Fence = device.createFence({vk::FenceCreateFlagBits::eSignaled});
      }

      ~VulkanFence() {
         m_Device.destroy(m_Fence);
      }

      vk::Fence GetVkFence() const {
         return m_Fence;
      }

   private:
      vk::Device m_Device;
      vk::Fence m_Fence;
   };

}
