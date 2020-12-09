#pragma once

#include "Pikzel/Renderer/Texture.h"
#include "VulkanDevice.h"
#include "VulkanImage.h"

#include <filesystem>

namespace Pikzel {

   TextureFormat VkFormatToTextureFormat(const vk::Format format);
   vk::Format TextureFormatToVkFormat(const TextureFormat format);

   class VulkanTexture2D : public Texture2D {
   public:
      VulkanTexture2D(std::shared_ptr<VulkanDevice> device, const uint32_t width, const uint32_t height, const TextureFormat format, const uint32_t mipLevels, vk::ImageUsageFlags usage = vk::ImageUsageFlagBits::eColorAttachment, vk::ImageAspectFlags aspect = vk::ImageAspectFlagBits::eColor);
      VulkanTexture2D(std::shared_ptr<VulkanDevice> device, const std::filesystem::path& path, const bool isSRGB);
      virtual ~VulkanTexture2D();

      virtual TextureFormat GetFormat() const override;
      virtual TextureType GetType() const override;

      virtual uint32_t GetWidth() const override;
      virtual uint32_t GetHeight() const override;

      virtual void SetData(void* data, const uint32_t size) override;

      virtual bool operator==(const Texture2D& that) override;

   public:
      vk::Sampler GetVkSampler() const;
      vk::ImageView GetVkImageView() const;
      vk::Format GetVkFormat() const;

      void TransitionImageLayout(vk::ImageLayout oldLayout, vk::ImageLayout newLayout);

   private:
      void CreateImage(const uint32_t width, const uint32_t height, const vk::Format format, const uint32_t mipLevels, vk::ImageUsageFlags usage, vk::ImageAspectFlags aspect);
      void DestroyImage();

      void CreateSampler();
      void DestroySampler();

   private:
      std::filesystem::path m_Path;
      std::shared_ptr<VulkanDevice> m_Device;
      std::unique_ptr<VulkanImage> m_Image;
      vk::Sampler m_TextureSampler;
   };


   class VulkanTextureCube : public TextureCube {
   public:

      VulkanTextureCube(std::shared_ptr<VulkanDevice> device, const uint32_t size, const TextureFormat format, const uint32_t mipLevels, vk::ImageUsageFlags usage = vk::ImageUsageFlagBits::eColorAttachment, vk::ImageAspectFlags aspect = vk::ImageAspectFlagBits::eColor);
      VulkanTextureCube(std::shared_ptr<VulkanDevice> device, const std::filesystem::path& path, const bool isSRGB);
      virtual ~VulkanTextureCube();

      virtual TextureFormat GetFormat() const override;
      virtual TextureType GetType() const override;

      virtual uint32_t GetWidth() const override;
      virtual uint32_t GetHeight() const override;

      virtual void SetData(void* data, const uint32_t size) override;

      bool operator==(const TextureCube& that) override;

   public:
      vk::Sampler GetVkSampler() const;
      vk::ImageView GetVkImageView() const;
      vk::Format GetVkFormat() const;

      void TransitionImageLayout(vk::ImageLayout oldLayout, vk::ImageLayout newLayout);

   private:
      void CreateImage(const uint32_t size, const vk::Format format, const uint32_t mipLevels, vk::ImageUsageFlags usage, vk::ImageAspectFlags aspect);
      void DestroyImage();

      void CreateSampler();
      void DestroySampler();

   private:
      std::filesystem::path m_Path;
      std::shared_ptr<VulkanDevice> m_Device;
      std::unique_ptr<VulkanImage> m_Image;
      vk::Sampler m_TextureSampler;

      // properties of underlying data that cube texture was created from
      TextureFormat m_DataFormat;
   };

}
