#pragma once

#include "VulkanDevice.h"

namespace Pikzel {

   class VulkanImage {
   public:

      VulkanImage(std::shared_ptr<VulkanDevice> device, const vk::ImageViewType type, const uint32_t width, const uint32_t height, const uint32_t layers, const uint32_t mipLevels, vk::SampleCountFlagBits numSamples, const vk::Format format, const vk::ImageTiling tiling, const vk::ImageUsageFlags usage, const vk::MemoryPropertyFlags properties);
      VulkanImage(std::shared_ptr<VulkanDevice> device, const vk::Image& image, vk::Format format, vk::Extent2D extent);
      VulkanImage(const VulkanImage&) = delete;   // You cannot copy Image wrapper object
      VulkanImage(VulkanImage&& that) noexcept = default;   // but you can move it (i.e. move the underlying vulkan resources to another Image wrapper)

      VulkanImage& operator=(const VulkanImage&) = delete;
      //VulkanImage& operator=(VulkanImage&& that) noexcept = default;

      virtual ~VulkanImage();

   public:
      vk::Image GetVkImage() const;
      vk::Format GetVkFormat() const;
      uint32_t GetWidth() const;
      uint32_t GetHeight() const;
      uint32_t GetDepth() const;
      uint32_t GetMIPLevels() const;
      uint32_t GetLayers() const;

      void CreateImageViews(const vk::Format format, const vk::ImageAspectFlags imageAspect);
      void DestroyImageViews();
      vk::ImageView GetVkImageView() const;
      vk::ImageView GetVkImageView(const uint32_t mipLevel) const;

      vk::ImageMemoryBarrier Barrier(const vk::ImageLayout oldLayout, const vk::ImageLayout newLayout, uint32_t baseMipLevel, uint32_t levelCount, uint32_t baseArrayLayer, uint32_t layerCount) const;

      void CopyFromBuffer(vk::Buffer buffer); // TODO: offsets
      void CopyFromImage(const VulkanImage& image, const vk::ImageCopy& region);

      void GenerateMipmap();

   protected:
      std::shared_ptr<VulkanDevice> m_Device;
      vk::ImageViewType m_Type;
      vk::Format m_Format;
      uint32_t m_Width;
      uint32_t m_Height;
      uint32_t m_Depth;
      uint32_t m_MIPLevels;
      uint32_t m_Layers;

      vk::Image m_Image;
      vk::DeviceMemory m_Memory;

      vk::ImageView m_ImageView;                    // all mip levels
      std::vector<vk::ImageView> m_MIPImageViews;   // individual mip levels
   };

}
