#pragma once

#include "VulkanDevice.h"

namespace Pikzel {

   class VulkanImage {
   public:

      VulkanImage(std::shared_ptr<VulkanDevice> device, const uint32_t width, const uint32_t height, const uint32_t mipLevels, vk::SampleCountFlagBits numSamples, const vk::Format format, const vk::ImageTiling tiling, const vk::ImageUsageFlags usage, const vk::MemoryPropertyFlags properties);
      VulkanImage(std::shared_ptr<VulkanDevice> device, const vk::Image& image, vk::Format format, vk::Extent2D extent);
      VulkanImage(const VulkanImage&) = delete;   // You cannot copy Image wrapper object
      VulkanImage(VulkanImage&& that) noexcept;   // but you can move it (i.e. move the underlying vulkan resources to another Image wrapper)

      VulkanImage& operator=(const VulkanImage&) = delete;
      VulkanImage& operator=(VulkanImage&& that) noexcept;

      virtual ~VulkanImage();

   public:
      vk::Format GetVkFormat() const;
      vk::Extent2D GetVkExtent() const;
      uint32_t GetMIPLevels() const;

      void CreateImageView(const vk::Format format, const vk::ImageAspectFlags imageAspect);
      void DestroyImageView();
      vk::ImageView GetVkImageView() const;

      void TransitionImageLayout(const vk::ImageLayout oldLayout, const vk::ImageLayout newLayout);

      void CopyFromBuffer(vk::Buffer buffer); // TODO: offsets

      void GenerateMIPMaps();

   protected:
      std::shared_ptr<VulkanDevice> m_Device;
      vk::Format m_Format;
      vk::Extent2D m_Extent;
      uint32_t m_MIPLevels;

      vk::Image m_Image;
      vk::DeviceMemory m_Memory;

      vk::ImageView m_ImageView;
   };

}
