#pragma once

#include "Pikzel/Renderer/Texture.h"
#include "VulkanDevice.h"
#include "VulkanImage.h"

#include <filesystem>

namespace Pikzel {

   TextureFormat VkFormatToTextureFormat(const vk::Format format);
   vk::Format TextureFormatToVkFormat(const TextureFormat format);
   vk::Filter TextureFilterToVkFilter(const TextureFilter filter);
   vk::SamplerMipmapMode TextureFilterToVkMipmapMode(const TextureFilter filter);
   vk::SamplerAddressMode TextureWrapToVkSamplerAddressMode(const TextureWrap wrap);

   class VulkanTexture : public Texture {
   public:
      virtual ~VulkanTexture();

      virtual uint32_t GetWidth() const override;
      virtual uint32_t GetHeight() const override;
      virtual uint32_t GetDepth() const override;
      virtual uint32_t GetLayers() const override;
      virtual uint32_t GetMIPLevels() const override;

      virtual TextureFormat GetFormat() const override;

      virtual void Commit(const uint32_t generateMipmapAfterLevel) override;

      virtual bool operator==(const Texture& that) override;

      void CopyFrom(const Texture& srcTexture, const TextureCopySettings& settings = {}) override;

   public:
      vk::Format GetVkFormat() const;
      vk::Image GetVkImage() const;
      vk::ImageView GetVkImageView() const;
      vk::ImageView GetVkImageView(const uint32_t mipLevel) const;
      vk::Sampler GetVkSampler() const;

      const VulkanImage& GetImage() const;

   protected:
      void Init(std::shared_ptr<VulkanDevice> device, const TextureSettings& settings, vk::ImageUsageFlags usage, vk::ImageAspectFlags aspect);

      virtual uint32_t CheckDepth(uint32_t depth) const = 0;
      virtual uint32_t CheckLayers(uint32_t layers) const = 0;

      void SetDataInternal(const uint32_t layer, const uint32_t slice, const uint32_t mipLevel, const uint32_t size, const void* data);

      void CreateImage(const vk::ImageViewType type, const uint32_t width, const uint32_t height, const uint32_t layers, const uint32_t mipLevels, const vk::Format format, vk::ImageUsageFlags usage, vk::ImageAspectFlags aspect);
      void DestroyImage();

      void CreateSampler(const TextureSettings& settings);
      void DestroySampler();

   protected:
      std::filesystem::path m_Path;
      std::shared_ptr<VulkanDevice> m_Device;
      std::unique_ptr<VulkanImage> m_Image;
      vk::Sampler m_TextureSampler;
      TextureFormat m_DataFormat; // this is used temporarily while uploading cubemap textures to GPU
   };


   class VulkanTexture2D : public VulkanTexture {
   public:
      VulkanTexture2D(std::shared_ptr<VulkanDevice> device, const TextureSettings& settings, vk::ImageUsageFlags usage = vk::ImageUsageFlagBits::eColorAttachment, vk::ImageAspectFlags aspect = vk::ImageAspectFlagBits::eColor);

      virtual TextureType GetType() const override;

      virtual void SetData(const void* data, const uint32_t size) override;

   protected:
      virtual uint32_t CheckDepth(uint32_t depth) const override;
      virtual uint32_t CheckLayers(uint32_t layers) const override;
   };


   class VulkanTexture2DArray : public VulkanTexture {
   public:
      VulkanTexture2DArray(std::shared_ptr<VulkanDevice> device, const TextureSettings& settings, vk::ImageUsageFlags usage = vk::ImageUsageFlagBits::eColorAttachment, vk::ImageAspectFlags aspect = vk::ImageAspectFlagBits::eColor);

      virtual TextureType GetType() const override;

      virtual void SetData(const void* data, const uint32_t size) override;

   protected:
      virtual uint32_t CheckDepth(uint32_t depth) const override;
      virtual uint32_t CheckLayers(uint32_t layers) const override;
   };


   class VulkanTextureCube : public VulkanTexture {
   public:
      VulkanTextureCube(std::shared_ptr<VulkanDevice> device, const TextureSettings& settings, vk::ImageUsageFlags usage = vk::ImageUsageFlagBits::eColorAttachment, vk::ImageAspectFlags aspect = vk::ImageAspectFlagBits::eColor);

      virtual TextureType GetType() const override;

      virtual void SetData(const void* data, const uint32_t size) override;

   protected:
      virtual uint32_t CheckDepth(uint32_t depth) const override;
      virtual uint32_t CheckLayers(uint32_t layers) const override;
   };


   class VulkanTextureCubeArray : public VulkanTexture {
   public:
      VulkanTextureCubeArray(std::shared_ptr<VulkanDevice> device, const TextureSettings& settings, vk::ImageUsageFlags usage = vk::ImageUsageFlagBits::eColorAttachment, vk::ImageAspectFlags aspect = vk::ImageAspectFlagBits::eColor);

      virtual TextureType GetType() const override;

      virtual void SetData(const void* data, const uint32_t size) override;

   protected:
      virtual uint32_t CheckDepth(uint32_t depth) const override;
      virtual uint32_t CheckLayers(uint32_t layers) const override;
   };

}
