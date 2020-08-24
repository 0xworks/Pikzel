#include "vkpch.h"
#include "VulkanImage.h"
#include "VulkanBuffer.h"
#include "imgui_impl_vulkan.h"

namespace Pikzel {

   VulkanImage::VulkanImage(std::shared_ptr<VulkanDevice> device, const uint32_t width, const uint32_t height, const uint32_t mipLevels, vk::SampleCountFlagBits numSamples, const vk::Format format, const vk::ImageTiling tiling, const vk::ImageUsageFlags usage, const vk::MemoryPropertyFlags properties)
   : m_Device {device}
   , m_Format {format}
   , m_Extent {width, height}
   , m_MIPLevels {mipLevels}
   {
      m_Image = m_Device->GetVkDevice().createImage({
         {}                               /*flags*/,
         vk::ImageType::e2D               /*imageType*/,
         format                           /*format*/,
         {width, height, 1}               /*extent*/,
         mipLevels                        /*mipLevels*/,
         1                                /*arrayLayers*/,
         numSamples                       /*samples*/,
         tiling                           /*tiling*/,
         usage                            /*usage*/,
         vk::SharingMode::eExclusive      /*sharingMode*/,
         0                                /*queueFamilyIndexCount*/,
         nullptr                          /*pQueueFamilyIndices*/,
         vk::ImageLayout::eUndefined      /*initialLayout*/
      });

      vk::MemoryRequirements memRequirements = m_Device->GetVkDevice().getImageMemoryRequirements(m_Image);
      m_Memory = m_Device->GetVkDevice().allocateMemory({
         memRequirements.size                                        /*allocationSize*/,
         VulkanBuffer::FindMemoryType(m_Device->GetVkPhysicalDevice(), memRequirements.memoryTypeBits, properties)  /*memoryTypeIndex*/
      });
      m_Device->GetVkDevice().bindImageMemory(m_Image, m_Memory, 0);
   }


   VulkanImage::VulkanImage(std::shared_ptr<VulkanDevice> device, const vk::Image& image, vk::Format format, vk::Extent2D extent)
   : m_Device {device}
   , m_Image {image}
   , m_Format {format}
   , m_Extent {extent}
   , m_MIPLevels {1}
   {}


   VulkanImage::VulkanImage(VulkanImage&& that) noexcept {
      *this = std::move(that);
   }


   VulkanImage::~VulkanImage() {
      if (m_Device) {
         if (m_Sampler) {
            m_Device->GetVkDevice().destroy(m_Sampler);
            m_Sampler = nullptr;
         }
         if (m_ImageView) {
            m_Device->GetVkDevice().destroy(m_ImageView);
            m_ImageView = nullptr;
         }
         if (m_Memory) {
            //
            // only destroy the image if it has some memory
            // i.e. we allocated the image, so we destroy it.
            // as opposed to images that were created (and are destroyed) by
            // the swap chain.
            if (m_Image) {
               m_Device->GetVkDevice().destroy(m_Image);
               m_Image = nullptr;
            }
            m_Device->GetVkDevice().freeMemory(m_Memory);
            m_Memory = nullptr;
         }
      }
   }


   vk::Format VulkanImage::GetFormat() const {
      return m_Format;
   }


   vk::Extent2D VulkanImage::GetExtent() const {
      return m_Extent;
   }


   uint32_t VulkanImage::GetMIPLevels() const {
      return m_MIPLevels;
   }


   VulkanImage& VulkanImage::operator=(VulkanImage&& that) noexcept {
      if (this != &that) {
         m_Device = that.m_Device;
         m_Image = that.m_Image;
         m_ImageView = that.m_ImageView;
         m_Memory = that.m_Memory;
         that.m_Device = nullptr;
         that.m_Image = nullptr;
         that.m_ImageView = nullptr;
         that.m_Memory = nullptr;
         m_Format = that.m_Format;
         m_Extent = that.m_Extent;
         m_MIPLevels = that.m_MIPLevels;
      }
      return *this;
   }


   void VulkanImage::CreateImageView(const vk::Format format, const vk::ImageAspectFlags imageAspect, const uint32_t mipLevels) {
      m_ImageView = m_Device->GetVkDevice().createImageView({
         {}                                 /*flags*/,
         m_Image                            /*image*/,
         vk::ImageViewType::e2D             /*viewType*/,
         format                             /*format*/,
         {}                                 /*components*/,
         {
            imageAspect                        /*aspectMask*/,
            0                                  /*baseMipLevel*/,
            mipLevels                          /*levelCount*/,
            0                                  /*baseArrayLevel*/,
            1                                  /*layerCount*/
         }                                  /*subresourceRange*/
      });
   }


   void VulkanImage::DestroyImageView() {
      if (m_Device && m_ImageView) {
         m_Device->GetVkDevice().destroy(m_ImageView);
         m_ImageView = nullptr;
      }
   }


   vk::ImageView VulkanImage::GetImageView() const {
      return m_ImageView;
   }


   void VulkanImage::CreateSampler() {
      m_Sampler = m_Device->GetVkDevice().createSampler({
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
         static_cast<float>(m_MIPLevels)     /*maxLod*/,
         vk::BorderColor::eFloatOpaqueBlack  /*borderColor*/,
         false                               /*unnormalizedCoordinates*/
      });
   }


   void VulkanImage::DestroySampler() {
      if (m_Device && m_Sampler) {
         m_Device->GetVkDevice().destroy(m_Sampler);
         m_Sampler = nullptr;
      }
   }


   vk::Sampler VulkanImage::GetSampler() const {
      return m_Sampler;
   }


   ImTextureID VulkanImage::GetImGuiTextureId() const {
      static ImTextureID texId = ImGui_ImplVulkan_AddTexture(m_Sampler, m_ImageView, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
      return texId;
   }

}