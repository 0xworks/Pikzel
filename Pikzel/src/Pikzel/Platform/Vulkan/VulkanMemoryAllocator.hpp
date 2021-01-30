#pragma once

#include "vk_mem_alloc.hpp"

namespace Pikzel {

   class VulkanMemoryAllocator {
      VulkanMemoryAllocator() = delete;

   public:
      static void Init(vk::Instance instance, vk::PhysicalDevice physicalDevice, vk::Device device) {
         vma::AllocatorCreateInfo allocatorInfo;
         allocatorInfo.vulkanApiVersion = VK_API_VERSION_1_1;
         allocatorInfo.instance = instance;
         allocatorInfo.physicalDevice = physicalDevice;
         allocatorInfo.device = device;
         m_Allocator = vma::createAllocator(allocatorInfo);
      }

      static vma::Allocator Get() {
         return m_Allocator;
      }

   private:
      inline static vma::Allocator m_Allocator = {};
   };

}
