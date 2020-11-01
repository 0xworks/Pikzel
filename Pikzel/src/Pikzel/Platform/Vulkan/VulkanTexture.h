#pragma once

#include "Pikzel/Renderer/Texture.h"
#include "VulkanDevice.h"
#include "VulkanImage.h"

#include <filesystem>

namespace Pikzel {

   class VulkanTexture2D : public Texture2D {
   public:
      VulkanTexture2D(std::shared_ptr<VulkanDevice> device, const uint32_t width, const uint32_t height, const vk::Format format);
      VulkanTexture2D(std::shared_ptr<VulkanDevice> device, const std::filesystem::path& path);
      virtual ~VulkanTexture2D();

      virtual uint32_t GetWidth() const override;
      virtual uint32_t GetHeight() const override;

      virtual void SetData(void* data, uint32_t size) override;

   public:
      vk::Sampler GetVkSampler() const;
      vk::ImageView GetVkImageView() const;
      vk::Format GetVkFormat() const;

   private:
      void CreateImage(const uint32_t width, const uint32_t height, const vk::Format format);
      void DestroyImage();

      void CreateSampler();
      void DestroySampler();

   private:
      std::filesystem::path m_Path;
      std::shared_ptr<VulkanDevice> m_Device;
      std::unique_ptr<VulkanImage> m_Image;
      vk::Sampler m_TextureSampler;
   };

}
