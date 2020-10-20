#include "VulkanTexture.h"

#include "VulkanBuffer.h"

#include <stb_image.h>

namespace Pikzel {


   VulkanTexture2D::VulkanTexture2D(std::shared_ptr<VulkanDevice> device, uint32_t width, uint32_t height)
   : m_Device {device} 
   {
      CreateImage(width, height);
      CreateSampler();
   }


   VulkanTexture2D::VulkanTexture2D(std::shared_ptr<VulkanDevice> device, const std::filesystem::path& path)
   : m_Path {path}
   , m_Device {device}
   {
      int width;
      int height;
      int channels;

      stbi_uc* pixels = stbi_load(path.string().data(), &width, &height, &channels, STBI_rgb_alpha);
      if (!pixels) {
         throw std::runtime_error(fmt::format("failed to load texture '{0}'", path.string()));
      }
      vk::DeviceSize size = static_cast<vk::DeviceSize>(width) * static_cast<vk::DeviceSize>(height) * 4;
      VulkanBuffer stagingBuffer(m_Device, size, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
      stagingBuffer.CopyFromHost(0, size, pixels);
      stbi_image_free(pixels);

      CreateImage(width, height);

      m_Image->TransitionImageLayout(vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal);
      m_Image->CopyFromBuffer(stagingBuffer.m_Buffer);
      m_Image->GenerateMIPMaps();

      CreateSampler();
   }


   VulkanTexture2D::~VulkanTexture2D() {
      DestroySampler();
      DestroyImage();
   }


   uint32_t VulkanTexture2D::GetWidth() const {
      return m_Image->GetExtent().width;
   }


   uint32_t VulkanTexture2D::GetHeight() const {
      return m_Image->GetExtent().height;
   }


   void VulkanTexture2D::SetData(void* data, uint32_t size) {
      VulkanBuffer stagingBuffer(m_Device, size, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
      stagingBuffer.CopyFromHost(0, size, data);
      m_Image->TransitionImageLayout(vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal);
      m_Image->CopyFromBuffer(stagingBuffer.m_Buffer);
      m_Image->GenerateMIPMaps();
   }


   vk::Sampler VulkanTexture2D::GetVkSampler() const {
      return m_TextureSampler;
   }


   vk::ImageView VulkanTexture2D::GetVkImageView() const {
      return m_Image->GetVkImageView();
   }


   void VulkanTexture2D::CreateImage(const uint32_t width, const uint32_t height) {
      uint32_t mipLevels = 1; // static_cast<uint32_t>(std::floor(std::log2(std::max(texWidth, texHeight)))) + 1;
      m_Image = std::make_unique<VulkanImage>(
         m_Device,
         width,
         height,
         mipLevels,
         vk::SampleCountFlagBits::e1,
         vk::Format::eR8G8B8A8Unorm,
         vk::ImageTiling::eOptimal,
         vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled,
         vk::MemoryPropertyFlagBits::eDeviceLocal
      );
      m_Image->CreateImageView(vk::Format::eR8G8B8A8Unorm, vk::ImageAspectFlagBits::eColor);
   }


   void VulkanTexture2D::DestroyImage() {
      m_Image = nullptr;
   }


   void VulkanTexture2D::CreateSampler() {
      m_TextureSampler = m_Device->GetVkDevice().createSampler({
         {}                                  /*flags*/,
         vk::Filter::eLinear                 /*magFilter*/,
         vk::Filter::eLinear                 /*minFilter*/,
         vk::SamplerMipmapMode::eLinear      /*mipmapMode*/,
         vk::SamplerAddressMode::eRepeat     /*addressModeU*/,
         vk::SamplerAddressMode::eRepeat     /*addressModeV*/,
         vk::SamplerAddressMode::eRepeat     /*addressModeW*/,
         0.0f                                /*mipLodBias*/,
         true                                /*anisotropyEnable*/,
         16                                  /*maxAnisotropy*/,
         false                               /*compareEnable*/,
         vk::CompareOp::eAlways              /*compareOp*/,
         0.0f                                /*minLod*/,
         static_cast<float>(1/*mipLevels*/)  /*maxLod*/,
         vk::BorderColor::eFloatOpaqueBlack  /*borderColor*/,
         false                               /*unnormalizedCoordinates*/
      });
   }


   void VulkanTexture2D::DestroySampler() {
      if (m_Device && m_TextureSampler) {
         m_Device->GetVkDevice().destroy(m_TextureSampler);
         m_TextureSampler = nullptr;
      }
   }
}
