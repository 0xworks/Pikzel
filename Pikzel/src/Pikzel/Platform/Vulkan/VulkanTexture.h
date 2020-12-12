#pragma once

#include "Pikzel/Renderer/Texture.h"
#include "VulkanDevice.h"
#include "VulkanImage.h"

#include <filesystem>

namespace Pikzel {

   TextureFormat VkFormatToTextureFormat(const vk::Format format);
   vk::Format TextureFormatToVkFormat(const TextureFormat format);


   class VulkanTexture : public Texture {
   public:
      virtual ~VulkanTexture();

      virtual uint32_t GetWidth() const override;
      virtual uint32_t GetHeight() const override;
      virtual uint32_t GetLayers() const override;

      virtual TextureFormat GetFormat() const override;

      virtual bool operator==(const Texture& that) override;

   public:
      vk::Sampler GetVkSampler() const;
      vk::ImageView GetVkImageView() const;
      vk::Format GetVkFormat() const;

      void TransitionImageLayout(vk::ImageLayout oldLayout, vk::ImageLayout newLayout);

   protected:
      void CreateImage(const vk::ImageViewType type, const uint32_t width, const uint32_t height, const uint32_t layers, const uint32_t mipLevels, const vk::Format format, vk::ImageUsageFlags usage, vk::ImageAspectFlags aspect);
      void DestroyImage();

      void CreateSampler();
      void DestroySampler();

   protected:
      std::shared_ptr<VulkanDevice> m_Device;
      std::unique_ptr<VulkanImage> m_Image;
      vk::Sampler m_TextureSampler;
   };


   class VulkanTexture2D : public VulkanTexture {
   public:
      VulkanTexture2D(std::shared_ptr<VulkanDevice> device, const uint32_t width, const uint32_t height, const TextureFormat format, const uint32_t mipLevels, vk::ImageUsageFlags usage = vk::ImageUsageFlagBits::eColorAttachment, vk::ImageAspectFlags aspect = vk::ImageAspectFlagBits::eColor);
      VulkanTexture2D(std::shared_ptr<VulkanDevice> device, const std::filesystem::path& path, const bool isSRGB);

      virtual TextureType GetType() const override;

      virtual void SetData(void* data, const uint32_t size) override;

   private:
      std::filesystem::path m_Path;
   };


   class VulkanTexture2DArray : public VulkanTexture {
   public:
      VulkanTexture2DArray(std::shared_ptr<VulkanDevice> device, const uint32_t width, const uint32_t height, const uint32_t layers, const TextureFormat format, const uint32_t mipLevels, vk::ImageUsageFlags usage = vk::ImageUsageFlagBits::eColorAttachment, vk::ImageAspectFlags aspect = vk::ImageAspectFlagBits::eColor);

      virtual TextureType GetType() const override;

      virtual void SetData(void* data, const uint32_t size) override;

   };


   class VulkanTextureCube : public VulkanTexture {
   public:

      VulkanTextureCube(std::shared_ptr<VulkanDevice> device, const uint32_t size, const TextureFormat format, const uint32_t mipLevels, vk::ImageUsageFlags usage = vk::ImageUsageFlagBits::eColorAttachment, vk::ImageAspectFlags aspect = vk::ImageAspectFlagBits::eColor);
      VulkanTextureCube(std::shared_ptr<VulkanDevice> device, const std::filesystem::path& path, const bool isSRGB);

      virtual TextureType GetType() const override;

      virtual void SetData(void* data, const uint32_t size) override;

   private:
      std::filesystem::path m_Path;
      TextureFormat m_DataFormat;
   };


   class VulkanTextureCubeArray : public VulkanTexture {
   public:

      VulkanTextureCubeArray(std::shared_ptr<VulkanDevice> device, const uint32_t size, const uint32_t layers, const TextureFormat format, const uint32_t mipLevels, vk::ImageUsageFlags usage = vk::ImageUsageFlagBits::eColorAttachment, vk::ImageAspectFlags aspect = vk::ImageAspectFlagBits::eColor);

      virtual TextureType GetType() const override;

      virtual void SetData(void* data, const uint32_t size) override;

   };

}
