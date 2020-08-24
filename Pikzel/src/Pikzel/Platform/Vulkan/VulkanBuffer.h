#pragma once

#include "Pikzel/Renderer/Buffer.h"
#include "VulkanDevice.h"

namespace Pikzel {

   class VulkanBuffer : public Buffer {
   public:

      VulkanBuffer(std::shared_ptr<VulkanDevice> device, const vk::DeviceSize size, const vk::BufferUsageFlags usage, const vk::MemoryPropertyFlags properties);
      VulkanBuffer(const VulkanBuffer&) = delete;   // You cannot copy Buffer wrapper object
      VulkanBuffer(VulkanBuffer&& that) noexcept;   // but you can move it (i.e. move the underlying vulkan resources to another Buffer wrapper)

      VulkanBuffer& operator=(const VulkanBuffer&) = delete;
      VulkanBuffer& operator=(VulkanBuffer&& that) noexcept;

      virtual ~VulkanBuffer();

      vk::DeviceSize m_Size;
      vk::BufferUsageFlags m_Usage;
      vk::MemoryPropertyFlags m_Properties;
      vk::Buffer m_Buffer;
      vk::DeviceMemory m_Memory;
      vk::DescriptorBufferInfo m_Descriptor;

      // Copy memory from host (pData) to the GPU buffer
      // You can do this only if buffer was created with host visible property
      virtual void CopyFromHost(const uint64_t offset, const uint64_t size, const void* pData) override;

   public:
      static uint32_t FindMemoryType(const vk::PhysicalDevice physicalDevice, const uint32_t typeFilter, const vk::MemoryPropertyFlags flags);

   protected:
      std::shared_ptr<VulkanDevice> m_Device;
   };

}