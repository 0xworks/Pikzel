#include "VulkanBuffer.h"
#include "VulkanUtility.h"

namespace Pikzel {

   VulkanBuffer::VulkanBuffer(std::shared_ptr<VulkanDevice> device, const vk::DeviceSize size, const vk::BufferUsageFlags usage, const vma::MemoryUsage memoryUsage)
   : m_Device{ device }
   , m_Size{ size }
   , m_Usage{ usage }
   {
      vk::BufferCreateInfo bufferInfo = {};
      bufferInfo.size = size;
      bufferInfo.usage = usage;

      vma::AllocationCreateInfo allocInfo = {};
      allocInfo.usage = memoryUsage;

      auto [buffer, allocation] = VulkanMemoryAllocator::Get().createBuffer(bufferInfo, allocInfo);
      m_Buffer = buffer;
      m_Allocation = allocation;

      m_Descriptor.buffer = m_Buffer;
      m_Descriptor.offset = 0;
      m_Descriptor.range = size;
   }


   VulkanBuffer::VulkanBuffer(VulkanBuffer&& that) noexcept {
      *this = std::move(that);
   }


   VulkanBuffer& VulkanBuffer::operator=(VulkanBuffer&& that) noexcept {
      if (this != &that) {
         if (m_Device && m_Buffer) {
            VulkanMemoryAllocator::Get().destroyBuffer(m_Buffer, m_Allocation);
            m_Buffer = nullptr;
            m_Allocation = nullptr;
         }
         m_Device = that.m_Device;
         m_Buffer = that.m_Buffer;
         m_Allocation = that.m_Allocation;
         m_Descriptor = that.m_Descriptor;
         m_Size = that.m_Size;
         m_Usage = that.m_Usage;
         that.m_Device = nullptr;
         that.m_Buffer = nullptr;
         that.m_Allocation = nullptr;
         that.m_Descriptor = vk::DescriptorBufferInfo{};
         that.m_Size = 0;
         that.m_Usage = {};
      }
      return *this;
   }


   VulkanBuffer::~VulkanBuffer() {
      if (m_Device && m_Buffer) {
         VulkanMemoryAllocator::Get().destroyBuffer(m_Buffer, m_Allocation);
         m_Buffer = nullptr;
         m_Allocation = nullptr;
      }
   }


   void VulkanBuffer::CopyFromHost(const uint64_t offset, const uint64_t size, const void* pData) {
      void* pDataDst = VulkanMemoryAllocator::Get().mapMemory(m_Allocation);
      memcpy(pDataDst, pData, static_cast<size_t>(size));
      VulkanMemoryAllocator::Get().unmapMemory(m_Allocation);
   }


   void VulkanBuffer::CopyFromBuffer(vk::Buffer src, const vk::DeviceSize srcOffset, const vk::DeviceSize dstOffset, const vk::DeviceSize size) {
      PKZL_ASSERT(dstOffset + size <= m_Size, "VulkanBuffer::CopyFromBuffer() buffer overrun!");
      m_Device->SubmitSingleTimeCommands(m_Device->GetTransferQueue(), [this, src, srcOffset, dstOffset, size] (vk::CommandBuffer cmd) {
         vk::BufferCopy copyRegion = {
            srcOffset,
            dstOffset,
            size
         };
         cmd.copyBuffer(src, m_Buffer, copyRegion);
      });
   }


   VulkanVertexBuffer::VulkanVertexBuffer(std::shared_ptr<VulkanDevice> device, const BufferLayout& layout, uint32_t size)
   : m_Buffer {device, size, vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer, vma::MemoryUsage::eGpuOnly}
   , m_Layout {layout}
   {}


   VulkanVertexBuffer::VulkanVertexBuffer(std::shared_ptr<VulkanDevice> device, const BufferLayout& layout, const uint32_t size, const void* data)
   : m_Buffer {device, size, vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer, vma::MemoryUsage::eGpuOnly}
   , m_Layout {layout}
   {
      CopyFromHost(0, size, data);
   }


   void VulkanVertexBuffer::CopyFromHost(const uint64_t offset, const uint64_t size, const void* pData) {
      VulkanBuffer stagingBuffer(m_Buffer.m_Device, size, vk::BufferUsageFlagBits::eTransferSrc, vma::MemoryUsage::eCpuToGpu);
      stagingBuffer.CopyFromHost(0, size, pData);
      m_Buffer.CopyFromBuffer(stagingBuffer.m_Buffer, 0, offset, size);
   }


   const BufferLayout& VulkanVertexBuffer::GetLayout() const {
      return m_Layout;
   }


   void VulkanVertexBuffer::SetLayout(const BufferLayout& layout) {
      PKZL_CORE_ASSERT(layout.GetElements().size(), "layout is empty!");
      m_Layout = layout;
   }


   vk::Buffer VulkanVertexBuffer::GetVkBuffer() const {
      return m_Buffer.m_Buffer;
   }


   VulkanIndexBuffer::VulkanIndexBuffer(std::shared_ptr<VulkanDevice> device, const uint32_t count, const uint32_t* indices)
   : m_Buffer {device, sizeof(uint32_t) * count, vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer, vma::MemoryUsage::eGpuOnly}
   , m_Count {count}
   {
      CopyFromHost(0, sizeof(uint32_t) * count, indices);
   }


   void VulkanIndexBuffer::CopyFromHost(const uint64_t offset, const uint64_t size, const void* pData) {
      VulkanBuffer stagingBuffer(m_Buffer.m_Device, size, vk::BufferUsageFlagBits::eTransferSrc, vma::MemoryUsage::eCpuToGpu);
      stagingBuffer.CopyFromHost(0, size, pData);
      m_Buffer.CopyFromBuffer(stagingBuffer.m_Buffer, 0, offset, size);
   }


   uint32_t VulkanIndexBuffer::GetCount() const {
      return m_Count;
   }


   vk::Buffer VulkanIndexBuffer::GetVkBuffer() const {
      return m_Buffer.m_Buffer;
   }


   VulkanUniformBuffer::VulkanUniformBuffer(std::shared_ptr<VulkanDevice> device, uint32_t size)
   : m_Buffer {device, size, vk::BufferUsageFlagBits::eUniformBuffer, vma::MemoryUsage::eCpuToGpu}
   {}


   VulkanUniformBuffer::VulkanUniformBuffer(std::shared_ptr<VulkanDevice> device, const uint32_t size, const void* data)
   : m_Buffer {device, size, vk::BufferUsageFlagBits::eUniformBuffer, vma::MemoryUsage::eCpuToGpu }
   {
      CopyFromHost(0, size, data);
   }


   void VulkanUniformBuffer::CopyFromHost(const uint64_t offset, const uint64_t size, const void* pData) {
      m_Buffer.CopyFromHost(0, size, pData);
   }


   vk::Buffer VulkanUniformBuffer::GetVkBuffer() const {
      return m_Buffer.m_Buffer;
   }

}
