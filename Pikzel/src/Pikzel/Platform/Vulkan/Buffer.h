#pragma once

#include <vulkan/vulkan.hpp>

namespace Pikzel {

   class Buffer {
   public:

      Buffer(vk::Device device, const vk::PhysicalDevice physicalDevice, const vk::DeviceSize size, const vk::BufferUsageFlags usage, const vk::MemoryPropertyFlags properties);
      Buffer(const Buffer&) = delete;   // You cannot copy Buffer wrapper object
      Buffer(Buffer&& that);            // but you can move it (i.e. move the underlying vulkan resources to another Buffer wrapper)

      Buffer& operator=(const Buffer&) = delete;
      Buffer& operator=(Buffer&& that);

      virtual ~Buffer();

      vk::DeviceSize m_Size;
      vk::BufferUsageFlags m_Usage;
      vk::MemoryPropertyFlags m_Properties;
      vk::Buffer m_Buffer;
      vk::DeviceMemory m_Memory;
      vk::DescriptorBufferInfo m_Descriptor;

      // Copy memory from host (pData) to the GPU buffer
      // You can do this only if buffer was created with host visible property
      void CopyFromHost(const vk::DeviceSize offset, const vk::DeviceSize size, const void* pData);

   public:
      static uint32_t FindMemoryType(const vk::PhysicalDevice physicalDevice, const uint32_t typeFilter, const vk::MemoryPropertyFlags flags);

   protected:
      vk::Device m_Device;
   };


   class IndexBuffer : public Buffer {
   public:

      IndexBuffer(vk::Device device, const vk::PhysicalDevice physicalDevice, const vk::DeviceSize size, const uint32_t count, const vk::BufferUsageFlags usage, const vk::MemoryPropertyFlags properties);

      uint32_t m_Count;
   };

}