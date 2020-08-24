#include "vkpch.h"
#include "VulkanBuffer.h"

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


   uint32_t VulkanBuffer::FindMemoryType(const vk::PhysicalDevice physicalDevice, const uint32_t typeFilter, const vk::MemoryPropertyFlags properties) {
      vk::PhysicalDeviceMemoryProperties memProperties = physicalDevice.getMemoryProperties();
      for (uint32_t i = 0; i < memProperties.memoryTypeCount; ++i) {
         if ((typeFilter & (1 << i)) && ((memProperties.memoryTypes[i].propertyFlags & properties) == properties)) {
            return i;
         }
      }
      throw std::runtime_error("Failed to find suitable memory type!");
   }


   void VulkanBuffer::CopyFromHost(const uint64_t offset, const uint64_t size, const void* pData) {
      void* pDataDst = m_Device->GetVkDevice().mapMemory(m_Memory, offset, size);
      memcpy(pDataDst, pData, static_cast<size_t>(size));
      m_Device->GetVkDevice().unmapMemory(m_Memory);
   }

}