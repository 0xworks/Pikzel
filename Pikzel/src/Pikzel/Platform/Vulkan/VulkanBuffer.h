#pragma once

#include "Pikzel/Renderer/Buffer.h"
#include "VulkanDevice.h"

namespace Pikzel {

   class VulkanBuffer {
   public:

      VulkanBuffer(std::shared_ptr<VulkanDevice> device, const vk::DeviceSize size, const vk::BufferUsageFlags usage, const vk::MemoryPropertyFlags properties);
      VulkanBuffer(const VulkanBuffer&) = delete;   // You cannot copy Buffer wrapper object
      VulkanBuffer(VulkanBuffer&& that) noexcept;   // but you can move it (i.e. move the underlying vulkan resources to another Buffer wrapper)

      VulkanBuffer& operator=(const VulkanBuffer&) = delete;
      VulkanBuffer& operator=(VulkanBuffer&& that) noexcept;

      virtual ~VulkanBuffer();

      // Copy memory from host (pData) to the GPU buffer
      // You can do this only if buffer was created with host visible property
      void CopyFromHost(const uint64_t offset, const uint64_t size, const void* pData);

      // Copy memory from GPU buffer
      void CopyFromBuffer(vk::Buffer src, const vk::DeviceSize srcOffset, const vk::DeviceSize dstOffset, const vk::DeviceSize size);

   public:
      vk::DescriptorBufferInfo m_Descriptor;
      std::shared_ptr<VulkanDevice> m_Device;
      vk::DeviceSize m_Size;
      vk::BufferUsageFlags m_Usage;
      vk::MemoryPropertyFlags m_Properties;
      vk::Buffer m_Buffer;
      vk::DeviceMemory m_Memory;
   

   };


   class VulkanVertexBuffer : public VertexBuffer {
   public:

      VulkanVertexBuffer(std::shared_ptr<VulkanDevice> device, uint32_t size);
      VulkanVertexBuffer(std::shared_ptr<VulkanDevice> device, float* vertices, uint32_t size);

      virtual void CopyFromHost(const uint64_t offset, const uint64_t size, const void* pData) override;

      const BufferLayout& GetLayout() const override;
      void SetLayout(const BufferLayout& layout) override;

      virtual void Bind() const override;
      virtual void Unbind() const override;

   private:
      VulkanBuffer m_Buffer;
   };


   class VulkanIndexBuffer : public IndexBuffer {
   public:

      VulkanIndexBuffer(std::shared_ptr<VulkanDevice> device, uint32_t* indices, uint32_t count);

      virtual void CopyFromHost(const uint64_t offset, const uint64_t size, const void* pData) override;

      virtual void Bind() const override;
      virtual void Unbind() const override;

      virtual uint32_t GetCount() const override;

   private:
      uint32_t m_Count;
      VulkanBuffer m_Buffer;
   };

}
