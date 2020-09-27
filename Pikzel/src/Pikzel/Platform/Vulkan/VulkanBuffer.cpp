#include "VulkanBuffer.h"
#include "VulkanUtility.h"

namespace Pikzel {

   VulkanBuffer::VulkanBuffer(std::shared_ptr<VulkanDevice> device, const vk::DeviceSize size, const vk::BufferUsageFlags usage, const vk::MemoryPropertyFlags properties)
   : m_Device {device}
   , m_Size {size}
   , m_Usage {usage}
   , m_Properties {properties}
   {
      m_Buffer = m_Device->GetVkDevice().createBuffer({
         {}                                         /*flags*/,
         size                                       /*size*/,
         usage                                      /*usage*/,
         vk::SharingMode::eExclusive                /*sharingMode*/,
         0                                          /*queueFamilyIndexCount*/,
         nullptr                                    /*pQueueFamilyIndices*/
      });

      const auto requirements = m_Device->GetVkDevice().getBufferMemoryRequirements(m_Buffer);
      m_Memory = m_Device->GetVkDevice().allocateMemory({
         requirements.size                                                        /*allocationSize*/,
         FindMemoryType(m_Device->GetVkPhysicalDevice(), requirements.memoryTypeBits, properties)  /*memoryTypeIndex*/
      });
      m_Device->GetVkDevice().bindBufferMemory(m_Buffer, m_Memory, 0);
      m_Descriptor.buffer = m_Buffer;
      m_Descriptor.offset = 0;
      m_Descriptor.range = size;
   }


   VulkanBuffer::VulkanBuffer(VulkanBuffer&& that) noexcept {
      *this = std::move(that);
   }


   VulkanBuffer& VulkanBuffer::operator=(VulkanBuffer&& that) noexcept {
      if (this != &that) {
         m_Device = that.m_Device;
         m_Buffer = that.m_Buffer;
         m_Memory = that.m_Memory;
         m_Descriptor = that.m_Descriptor;
         m_Size = that.m_Size;
         m_Usage = that.m_Usage;
         m_Properties = that.m_Properties;
         that.m_Device = nullptr;
         that.m_Buffer = nullptr;
         that.m_Memory = nullptr;
         that.m_Descriptor = {};
         that.m_Size = 0;
         that.m_Usage = {};
         that.m_Properties = {};
      }
      return *this;
   }


   VulkanBuffer::~VulkanBuffer() {
      if (m_Device) {
         if (m_Buffer) {
            m_Device->GetVkDevice().destroy(m_Buffer);
            m_Buffer = nullptr;
         }
         if (m_Memory) {
            m_Device->GetVkDevice().freeMemory(m_Memory);
            m_Memory = nullptr;
         }
      }
   }


   void VulkanBuffer::CopyFromHost(const uint64_t offset, const uint64_t size, const void* pData) {
      void* pDataDst = m_Device->GetVkDevice().mapMemory(m_Memory, offset, size);
      memcpy(pDataDst, pData, static_cast<size_t>(size));
      m_Device->GetVkDevice().unmapMemory(m_Memory);
   }


   void VulkanBuffer::CopyFromBuffer(vk::Buffer src, const vk::DeviceSize srcOffset, const vk::DeviceSize dstOffset, const vk::DeviceSize size) {
      PKZL_ASSERT(dstOffset + size <= m_Size, "VulkanBuffer::CopyFromBuffer() buffer overrun!");
      m_Device->SubmitSingleTimeCommands([this, src, srcOffset, dstOffset, size] (vk::CommandBuffer cmd) {
         vk::BufferCopy copyRegion = {
            srcOffset,
            dstOffset,
            size
         };
         cmd.copyBuffer(src, m_Buffer, copyRegion);
      });
   }


   VulkanVertexBuffer::VulkanVertexBuffer(std::shared_ptr<VulkanDevice> device, uint32_t size)
   : m_Buffer {device, size, vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer, vk::MemoryPropertyFlagBits::eDeviceLocal}
   {}


   VulkanVertexBuffer::VulkanVertexBuffer(std::shared_ptr<VulkanDevice> device, const uint32_t size, const void* data)
   : m_Buffer {device, size, vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer, vk::MemoryPropertyFlagBits::eDeviceLocal}
   {
      CopyFromHost(0, size, data);
   }


   void VulkanVertexBuffer::CopyFromHost(const uint64_t offset, const uint64_t size, const void* pData) {
      VulkanBuffer stagingBuffer(m_Buffer.m_Device, size, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
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
   : m_Buffer {device, sizeof(uint32_t) * count, vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer, vk::MemoryPropertyFlagBits::eDeviceLocal}
   , m_Count {count}
   {
      CopyFromHost(0, sizeof(uint32_t) * count, indices);
   }


   void VulkanIndexBuffer::CopyFromHost(const uint64_t offset, const uint64_t size, const void* pData) {
      VulkanBuffer stagingBuffer(m_Buffer.m_Device, size, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
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
      : m_Buffer {device, size, vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eUniformBuffer, vk::MemoryPropertyFlagBits::eDeviceLocal} {
   }


   VulkanUniformBuffer::VulkanUniformBuffer(std::shared_ptr<VulkanDevice> device, const uint32_t size, const void* data)
   : m_Buffer {device, size, vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eUniformBuffer, vk::MemoryPropertyFlagBits::eDeviceLocal} {
      CopyFromHost(0, size, data);
   }


   void VulkanUniformBuffer::CopyFromHost(const uint64_t offset, const uint64_t size, const void* pData) {
      VulkanBuffer stagingBuffer(m_Buffer.m_Device, size, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
      stagingBuffer.CopyFromHost(0, size, pData);
      m_Buffer.CopyFromBuffer(stagingBuffer.m_Buffer, 0, offset, size);
   }


   vk::Buffer VulkanUniformBuffer::GetVkBuffer() const {
      return m_Buffer.m_Buffer;
   }
         
}
