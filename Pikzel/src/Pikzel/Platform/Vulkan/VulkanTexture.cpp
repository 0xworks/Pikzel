#include "VulkanTexture.h"

#include "VulkanBuffer.h"
#include "VulkanPipeline.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace Pikzel {

   TextureFormat VkFormatToTextureFormat(const vk::Format format) {
      switch (format) {
         // case vk::Format::eR8G8B8Unorm: return TextureFormat::RGB8; // not supported.  need 32-bits per texel
         case vk::Format::eR8G8B8A8Unorm:         return TextureFormat::RGBA8;
         // case vk::Format::eR8G8B8Srgb: return TextureFormat::SRGB8; // not supported.  need 32-bits per texel
         case vk::Format::eR8G8B8A8Srgb:          return TextureFormat::SRGB8;
         case vk::Format::eR16G16B16Sfloat:       return TextureFormat::RGB16F;
         case vk::Format::eR16G16B16A16Sfloat:    return TextureFormat::RGBA16F;
         case vk::Format::eR32G32B32Sfloat:       return TextureFormat::RGB32F;
         case vk::Format::eR32G32B32A32Sfloat:    return TextureFormat::RGBA32F;
         case vk::Format::eB10G11R11UfloatPack32: return TextureFormat::BGR8;
         case vk::Format::eB8G8R8A8Srgb:          return TextureFormat::BGRA8;
         case vk::Format::eR8Unorm:               return TextureFormat::R8;
         case vk::Format::eR32Sfloat:             return TextureFormat::R32F;
         case vk::Format::eD32Sfloat:             return TextureFormat::D32F;
         case vk::Format::eD24UnormS8Uint:        return TextureFormat::D24S8;
         case vk::Format::eD32SfloatS8Uint:       return TextureFormat::D32S8;
      }
      PKZL_CORE_ASSERT(false, "Unsupported TextureFormat!");
      return TextureFormat::Undefined;
   }


   vk::Format TextureFormatToVkFormat(const TextureFormat format) {
      switch (format) {
         // case TextureFormat::RGB8:  return vk::Format::eR8G8B8Unorm; // not supported.  need 32-bits per texel
         case TextureFormat::RGBA8:    return vk::Format::eR8G8B8A8Unorm;
         // case TextureFormat::SRGB8: return vk::Format::eR8G8B8Srgb; // not supported.  need 32-bits per texel
         case TextureFormat::SRGBA8:   return vk::Format::eR8G8B8A8Srgb;
         case TextureFormat::RGB16F:   return vk::Format::eR16G16B16Sfloat;
         case TextureFormat::RGBA16F:  return vk::Format::eR16G16B16A16Sfloat;
         case TextureFormat::RGB32F:   return vk::Format::eR32G32B32Sfloat;
         case TextureFormat::RGBA32F:  return vk::Format::eR32G32B32A32Sfloat;
         case TextureFormat::BGR8:     return vk::Format::eB10G11R11UfloatPack32; // the texels must still be 32-bits, so if no alpha channel then use more bits in other channels
         case TextureFormat::BGRA8:    return vk::Format::eB8G8R8A8Srgb;
         case TextureFormat::R8:       return vk::Format::eR8Unorm;
         case TextureFormat::R32F:     return vk::Format::eR32Sfloat;
         case TextureFormat::D32F:     return vk::Format::eD32Sfloat;
         case TextureFormat::D24S8:    return vk::Format::eD24UnormS8Uint;
         case TextureFormat::D32S8:    return vk::Format::eD32SfloatS8Uint;
      }
      PKZL_CORE_ASSERT(false, "Unsupported TextureFormat!");
      return vk::Format::eUndefined;
   }


   static stbi_uc* STBILoad(const std::filesystem::path& path, const bool isSRGB, uint32_t* width, uint32_t* height, TextureFormat* format) {
      int iWidth;
      int iHeight;
      int channels;
      stbi_set_flip_vertically_on_load(1);
      stbi_uc* data = nullptr;
      bool isHDR = stbi_is_hdr(path.string().c_str());
      if (isHDR) {
         data = reinterpret_cast<stbi_uc*>(stbi_loadf(path.string().c_str(), &iWidth, &iHeight, &channels, STBI_rgb_alpha));
         if ((channels == 1) || (channels == 2)) {
            // if it was a 1 or 2 channel texture, then we should reload it with the actual number of channels
            // instead of forcing 4
            // (unfortunately no way to determine channels before loading the whole thing!)
            data = reinterpret_cast<stbi_uc*>(stbi_loadf(path.string().c_str(), &iWidth, &iHeight, &channels, 0));
         }
      } else {
         data = stbi_load(path.string().c_str(), &iWidth, &iHeight, &channels, STBI_rgb_alpha);
         if ((channels == 1) || (channels == 2)) {
            // if it was a 1 or 2 channel texture, then we should reload it with the actual number of channels
            // instead of forcing 4
            // (unfortunately no way to determine channels before loading the whole thing!)
            data = stbi_load(path.string().c_str(), &iWidth, &iHeight, &channels, 0);
         }
      }

      if (!data) {
         throw std::runtime_error {fmt::format("failed to load image '{0}'", path.string())};
      }
      *width = static_cast<uint32_t>(iWidth);
      *height = static_cast<uint32_t>(iHeight);

      if ((channels == 3) || (channels == 4)) {
         // we forced in an alpha channel in the load above (because we don't have 24-bit textures here)
         *format = isHDR ? TextureFormat::RGBA32F : isSRGB ? TextureFormat::SRGBA8 : TextureFormat::RGBA8;
      } else if(channels == 1) {
         *format = isHDR ? TextureFormat::R32F : TextureFormat::R8;
      }
      else {
         throw std::runtime_error {fmt::format("'{0}': Image format not supported!", path.string())};
      }

      return data;
   }


   VulkanTexture::~VulkanTexture() {
      DestroySampler();
      DestroyImage();
   }


   uint32_t VulkanTexture::GetWidth() const {
      return m_Image->GetWidth();
   }


   uint32_t VulkanTexture::GetHeight() const {
      return m_Image->GetHeight();
   }


   uint32_t VulkanTexture::GetLayers() const {
      return m_Image->GetLayers();
   }


   TextureFormat VulkanTexture::GetFormat() const {
      return VkFormatToTextureFormat(m_Image->GetVkFormat());
   }


   bool VulkanTexture::operator==(const Texture& that) {
      return m_Image->GetVkImage() == static_cast<const VulkanTexture&>(that).m_Image->GetVkImage();
   }


   vk::Sampler VulkanTexture::GetVkSampler() const {
      return m_TextureSampler;
   }


   vk::ImageView VulkanTexture::GetVkImageView() const {
      return m_Image->GetVkImageView();
   }


   vk::Format VulkanTexture::GetVkFormat() const {
      return m_Image->GetVkFormat();
   }


   void VulkanTexture::TransitionImageLayout(vk::ImageLayout oldLayout, vk::ImageLayout newLayout) {
      m_Image->TransitionImageLayout(oldLayout, newLayout);
   }


   void VulkanTexture::CreateImage(const vk::ImageViewType type, const uint32_t width, const uint32_t height, const uint32_t layers, const uint32_t mipLevels, const vk::Format format, vk::ImageUsageFlags usage, vk::ImageAspectFlags aspect) {
      m_Image = std::make_unique<VulkanImage>(
         m_Device,
         type,
         width,
         height,
         layers,
         mipLevels,
         vk::SampleCountFlagBits::e1,
         format,
         vk::ImageTiling::eOptimal,
         usage | vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled,
         vk::MemoryPropertyFlagBits::eDeviceLocal
      );
      m_Image->CreateImageView(format, aspect);
   }


   void VulkanTexture::DestroyImage() {
      m_Image.reset();
   }


   void VulkanTexture::CreateSampler() {
      if (IsDepthFormat(GetFormat())) {
         m_TextureSampler = m_Device->GetVkDevice().createSampler({
            {}                                                              /*flags*/,
            vk::Filter::eNearest                                            /*magFilter*/,
            vk::Filter::eNearest                                            /*minFilter*/,
            vk::SamplerMipmapMode::eNearest                                 /*mipmapMode*/,
            vk::SamplerAddressMode::eClampToEdge                            /*addressModeU*/,
            vk::SamplerAddressMode::eClampToEdge                            /*addressModeV*/,
            vk::SamplerAddressMode::eClampToEdge                            /*addressModeW*/,
            0.0f                                                            /*mipLodBias*/,
            m_Device->GetEnabledPhysicalDeviceFeatures().samplerAnisotropy  /*anisotropyEnable*/,
            16                                                              /*maxAnisotropy*/,
            false                                                           /*compareEnable*/,
            vk::CompareOp::eNever                                           /*compareOp*/,
            0.0f                                                            /*minLod*/,
            static_cast<float>(m_Image->GetMIPLevels())                     /*maxLod*/,
            vk::BorderColor::eFloatOpaqueBlack                              /*borderColor*/,
            false                                                           /*unnormalizedCoordinates*/
         });
      } else {
         m_TextureSampler = m_Device->GetVkDevice().createSampler({
            {}                                                              /*flags*/,
            vk::Filter::eLinear                                             /*magFilter*/,
            vk::Filter::eLinear                                             /*minFilter*/,
            vk::SamplerMipmapMode::eLinear                                  /*mipmapMode*/,
            vk::SamplerAddressMode::eRepeat                                 /*addressModeU*/,
            vk::SamplerAddressMode::eRepeat                                 /*addressModeV*/,
            vk::SamplerAddressMode::eRepeat                                 /*addressModeW*/,
            0.0f                                                            /*mipLodBias*/,
            m_Device->GetEnabledPhysicalDeviceFeatures().samplerAnisotropy  /*anisotropyEnable*/,
            16                                                              /*maxAnisotropy*/,
            false                                                           /*compareEnable*/,
            vk::CompareOp::eNever                                           /*compareOp*/,
            0.0f                                                            /*minLod*/,
            static_cast<float>(m_Image->GetMIPLevels())                     /*maxLod*/,
            vk::BorderColor::eFloatOpaqueBlack                              /*borderColor*/,
            false                                                           /*unnormalizedCoordinates*/
         });
      }
   }


   void VulkanTexture::DestroySampler() {
      if (m_Device && m_TextureSampler) {
         m_Device->GetVkDevice().destroy(m_TextureSampler);
         m_TextureSampler = nullptr;
      }
   }


   VulkanTexture2D::VulkanTexture2D(std::shared_ptr<VulkanDevice> device, const uint32_t width, const uint32_t height, const TextureFormat format, const uint32_t mipLevels, vk::ImageUsageFlags usage, vk::ImageAspectFlags aspect) {
      m_Device = device;
      CreateImage(vk::ImageViewType::e2D, width, height, 1, mipLevels, TextureFormatToVkFormat(format), usage, aspect);
      CreateSampler();
   }


   VulkanTexture2D::VulkanTexture2D(std::shared_ptr<VulkanDevice> device, const std::filesystem::path& path, const bool isSRGB)
   : m_Path {path}
   {
      uint32_t width;
      uint32_t height;
      TextureFormat format;
      stbi_uc* data = STBILoad(path, isSRGB, &width, &height, &format);

      m_Device = device;
      CreateImage(vk::ImageViewType::e2D, width, height, 1, CalculateMipMapLevels(width, height), TextureFormatToVkFormat(format), vk::ImageUsageFlagBits::eColorAttachment, vk::ImageAspectFlagBits::eColor);
      CreateSampler();
      PKZL_CORE_ASSERT(BPP(format) != 3, "VulkanTexture2D format cannot be 24-bits per texel");
      SetData(data, width * height * BPP(format));

      stbi_image_free(data);
   }


   TextureType VulkanTexture2D::GetType() const {
      return TextureType::Texture2D;
   }


   void VulkanTexture2D::SetData(void* data, uint32_t size) {
      VulkanBuffer stagingBuffer(m_Device, size, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
      stagingBuffer.CopyFromHost(0, size, data);
      m_Image->TransitionImageLayout(vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal);
      m_Image->CopyFromBuffer(stagingBuffer.m_Buffer);
      m_Image->GenerateMIPMaps();
   }


   VulkanTexture2DArray::VulkanTexture2DArray(std::shared_ptr<VulkanDevice> device, const uint32_t width, const uint32_t height, const uint32_t layers, const TextureFormat format, const uint32_t mipLevels, vk::ImageUsageFlags usage /*= vk::ImageUsageFlagBits::eColorAttachment*/, vk::ImageAspectFlags aspect /*= vk::ImageAspectFlagBits::eColor*/) {
      m_Device = device;
      CreateImage(vk::ImageViewType::e2DArray, width, height, layers, mipLevels, TextureFormatToVkFormat(format), usage, aspect);
      CreateSampler();
   }


   TextureType VulkanTexture2DArray::GetType() const {
      return TextureType::Texture2DArray;
   }


   void VulkanTexture2DArray::SetData(void* data, const uint32_t size) {
      VulkanBuffer stagingBuffer(m_Device, size, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
      stagingBuffer.CopyFromHost(0, size, data);
      m_Image->TransitionImageLayout(vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal);
      m_Image->CopyFromBuffer(stagingBuffer.m_Buffer);
      m_Image->GenerateMIPMaps();
   }


   VulkanTextureCube::VulkanTextureCube(std::shared_ptr<VulkanDevice> device, const uint32_t size, const TextureFormat format, const uint32_t mipLevels, vk::ImageUsageFlags usage, vk::ImageAspectFlags aspect) {
      m_Device = device;
      CreateImage(vk::ImageViewType::eCube, size, size, 1, mipLevels, TextureFormatToVkFormat(format), usage, aspect);
      CreateSampler();
   }


   VulkanTextureCube::VulkanTextureCube(std::shared_ptr<VulkanDevice> device, const std::filesystem::path& path, const bool isSRGB)
   : m_Path {path}
   {
      uint32_t width;
      uint32_t height;
      stbi_uc* data = STBILoad(path, isSRGB, &width, &height, &m_DataFormat);

      // guess whether the data is the 6-faces of a cube, or whether it's equirectangular
      // width is twice the height -> equirectangular (probably)
      // width is 4/3 the height -> 6 faces of a cube (probably)
      uint32_t size = 0;
      if (width / 2 == height) {
         size = height;
      } else {
         size = width / 4;
      }

      m_Device = device;
      CreateImage(vk::ImageViewType::eCube, size, size, 1, CalculateMipMapLevels(size, size), TextureFormatToVkFormat(TextureFormat::RGBA8), vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eStorage, vk::ImageAspectFlagBits::eColor);
      CreateSampler();
      PKZL_CORE_ASSERT(BPP(m_DataFormat) != 3, "VulkanTextureCube format cannot be 24-bits per texel");
      SetData(data, width * height * BPP(m_DataFormat));
      stbi_image_free(data);
   }


   TextureType VulkanTextureCube::GetType() const {
      return TextureType::TextureCube;
   }


   void VulkanTextureCube::SetData(void* data, const uint32_t size) {
      uint32_t width = GetWidth();
      uint32_t height = GetHeight();
      const char* shader = nullptr;
      if (size == (width * 2 * height * BPP(m_DataFormat))) {
         width *= 2;
         shader = "Renderer/EquirectangularToCubeMap.comp.spv";
      } else if (size == width * 4 * height * 3 * BPP(m_DataFormat)) {
         width *= 4;
         height *= 3;
         shader = "Renderer/SixFacesToCubeMap.comp.spv";
      } else {
         throw std::runtime_error("Data must be entire texture!");
      }

      std::unique_ptr<VulkanTexture2D> tex2d = std::make_unique<VulkanTexture2D>(m_Device, width, height, m_DataFormat, 1);
      tex2d->SetData(data, size);
      int tonemap = 0;
      if (
         (tex2d->GetFormat() == TextureFormat::RGB32F) ||
         (tex2d->GetFormat() == TextureFormat::RGBA32F)
      ) {
         tonemap = 1;
      }

      m_Image->TransitionImageLayout(vk::ImageLayout::eUndefined, vk::ImageLayout::eGeneral);

      std::unique_ptr<VulkanPipeline> pipeline = std::make_unique<VulkanPipeline>(
         m_Device,
         PipelineSettings {
            {},
            {
               { Pikzel::ShaderType::Compute, shader }
            }
         }
      );

      const VulkanResource& texResource = pipeline->GetResource("uTexture"_hs);
      vk::DescriptorImageInfo texImageDescriptor = {
            tex2d->GetVkSampler()                    /*sampler*/,
            tex2d->GetVkImageView()                  /*imageView*/,
            vk::ImageLayout::eShaderReadOnlyOptimal  /*imageLayout*/
      };

      const VulkanResource& cubeResource = pipeline->GetResource("outCubeMap"_hs);
      vk::DescriptorImageInfo cubeImageDescriptor = {
            nullptr                                  /*sampler*/,
            GetVkImageView()                         /*imageView*/,
            vk::ImageLayout::eGeneral                /*imageLayout*/
      };

      std::array<vk::WriteDescriptorSet, 2> writeDescriptors = {
         vk::WriteDescriptorSet {
            pipeline->GetVkDescriptorSet(texResource.DescriptorSet)  /*dstSet*/,
            texResource.Binding                                      /*dstBinding*/,
            0                                                        /*dstArrayElement*/,
            texResource.GetCount()                                   /*descriptorCount*/,
            texResource.Type                                         /*descriptorType*/,
            &texImageDescriptor                                      /*pImageInfo*/,
            nullptr                                                  /*pBufferInfo*/,
            nullptr                                                  /*pTexelBufferView*/
         },
         vk::WriteDescriptorSet {
            pipeline->GetVkDescriptorSet(cubeResource.DescriptorSet)  /*dstSet*/,
            cubeResource.Binding                                      /*dstBinding*/,
            0                                                         /*dstArrayElement*/,
            cubeResource.GetCount()                                   /*descriptorCount*/,
            cubeResource.Type                                         /*descriptorType*/,
            &cubeImageDescriptor                                      /*pImageInfo*/,
            nullptr                                                   /*pBufferInfo*/,
            nullptr                                                   /*pTexelBufferView*/
         }
      };

      m_Device->GetVkDevice().updateDescriptorSets(writeDescriptors, nullptr);

      m_Device->SubmitSingleTimeCommands(m_Device->GetComputeQueue(), [this, &pipeline, &tex2d, tonemap] (vk::CommandBuffer cmd) {
         cmd.bindPipeline(vk::PipelineBindPoint::eCompute, pipeline->GetVkPipelineCompute());
         pipeline->BindDescriptorSets(cmd, nullptr);

         const VulkanPushConstant& tonemapC = pipeline->GetPushConstant("constants.tonemap"_hs);
         PKZL_CORE_ASSERT(tonemapC.Type == DataType::Int, "Push constant '{0}' type mismatch.  Was {1}, expected int!", tonemapC.Name, DataTypeToString(tonemapC.Type));
         cmd.pushConstants<int>(pipeline->GetVkPipelineLayout(), tonemapC.ShaderStages, tonemapC.Offset, tonemap);

         cmd.dispatch(GetWidth() / 32, GetHeight() / 32, 6);
      });
      pipeline->UnbindDescriptorSets();

      m_Image->TransitionImageLayout(vk::ImageLayout::eGeneral, vk::ImageLayout::eTransferDstOptimal);
      m_Image->GenerateMIPMaps();
   }


   VulkanTextureCubeArray::VulkanTextureCubeArray(std::shared_ptr<VulkanDevice> device, const uint32_t size, const uint32_t layers, const TextureFormat format, const uint32_t mipLevels, vk::ImageUsageFlags usage /*= vk::ImageUsageFlagBits::eColorAttachment*/, vk::ImageAspectFlags aspect /*= vk::ImageAspectFlagBits::eColor*/) {
      m_Device = device;
      CreateImage(vk::ImageViewType::eCubeArray, size, size, layers, mipLevels, TextureFormatToVkFormat(format), usage, aspect);
      CreateSampler();
   }


   TextureType VulkanTextureCubeArray::GetType() const {
      return TextureType::TextureCubeArray;
   }


   void VulkanTextureCubeArray::SetData(void* data, const uint32_t size) {
      PKZL_NOT_IMPLEMENTED;
   }

}
