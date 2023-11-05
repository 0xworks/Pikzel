#include "VulkanTexture.h"

#include "VulkanBuffer.h"
#include "VulkanComputeContext.h"
#include "VulkanPipeline.h"

namespace Pikzel {

   vk::ImageViewType TextureTypeToVkImageViewType(const TextureType& type) {
      switch (type) {
         case TextureType::Texture2D:        return vk::ImageViewType::e2D;
         case TextureType::Texture2DArray:   return vk::ImageViewType::e2DArray;
         case TextureType::TextureCube:      return vk::ImageViewType::eCube;
         case TextureType::TextureCubeArray: return vk::ImageViewType::eCubeArray;
         default:                            break;
      }
      PKZL_CORE_ASSERT(false, "Unsupported TextureType!");
      return {};
   }


   TextureFormat VkFormatToTextureFormat(const vk::Format format) {
      switch (format) {
         // case vk::Format::eR8G8B8Unorm:        return TextureFormat::RGB8; // not supported.  need 32-bits per texel
         case vk::Format::eR8G8B8A8Unorm:         return TextureFormat::RGBA8;
         // case vk::Format::eR8G8B8Srgb:         return TextureFormat::SRGB8; // not supported.  need 32-bits per texel
         case vk::Format::eR8G8B8A8Srgb:          return TextureFormat::SRGB8;
         case vk::Format::eR16G16Sfloat:          return TextureFormat::RG16F;
         case vk::Format::eR16G16B16Sfloat:       return TextureFormat::RGB16F;
         case vk::Format::eR16G16B16A16Sfloat:    return TextureFormat::RGBA16F;
         case vk::Format::eR32G32Sfloat:          return TextureFormat::RG32F;
         case vk::Format::eR32G32B32Sfloat:       return TextureFormat::RGB32F;
         case vk::Format::eR32G32B32A32Sfloat:    return TextureFormat::RGBA32F;
         case vk::Format::eB10G11R11UfloatPack32: return TextureFormat::BGR8;
         case vk::Format::eB8G8R8A8Srgb:          return TextureFormat::BGRA8;
         case vk::Format::eR8Unorm:               return TextureFormat::R8;
         case vk::Format::eR32Sfloat:             return TextureFormat::R32F;
         case vk::Format::eD32Sfloat:             return TextureFormat::D32F;
         case vk::Format::eD24UnormS8Uint:        return TextureFormat::D24S8;
         case vk::Format::eD32SfloatS8Uint:       return TextureFormat::D32S8;
         case vk::Format::eBc1RgbaUnormBlock:     return TextureFormat::DXT1RGBA;
         case vk::Format::eBc1RgbaSrgbBlock:      return TextureFormat::DXT1SRGBA;
         case vk::Format::eBc2UnormBlock:         return TextureFormat::DXT3RGBA;
         case vk::Format::eBc2SrgbBlock:          return TextureFormat::DXT3SRGBA;
         case vk::Format::eBc3UnormBlock:         return TextureFormat::DXT5RGBA;
         case vk::Format::eBc3SrgbBlock:          return TextureFormat::DXT5SRGBA;
         case vk::Format::eBc4UnormBlock:         return TextureFormat::RGTC1R;
         case vk::Format::eBc4SnormBlock:         return TextureFormat::RGTC1SR;
         case vk::Format::eBc5UnormBlock:         return TextureFormat::RGTC2RG;
         case vk::Format::eBc5SnormBlock:         return TextureFormat::RGTC2SRG;
         default:                                 break;
      }
      PKZL_CORE_ASSERT(false, "Unsupported TextureFormat!");
      return TextureFormat::Undefined;
   }


   vk::Format TextureFormatToVkFormat(const TextureFormat format) {
      switch (format) {
         // case TextureFormat::RGB8:     return vk::Format::eR8G8B8Unorm; // not supported.  need 32-bits per texel
         case TextureFormat::RGBA8:       return vk::Format::eR8G8B8A8Unorm;
         // case TextureFormat::SRGB8:    return vk::Format::eR8G8B8Srgb; // not supported.  need 32-bits per texel
         case TextureFormat::SRGBA8:      return vk::Format::eR8G8B8A8Srgb;
         case TextureFormat::RG16F:       return vk::Format::eR16G16Sfloat;
         case TextureFormat::RGB16F:      return vk::Format::eR16G16B16Sfloat;
         case TextureFormat::RGBA16F:     return vk::Format::eR16G16B16A16Sfloat;
         case TextureFormat::RG32F:       return vk::Format::eR32G32Sfloat;
         case TextureFormat::RGB32F:      return vk::Format::eR32G32B32Sfloat;
         case TextureFormat::RGBA32F:     return vk::Format::eR32G32B32A32Sfloat;
         case TextureFormat::BGR8:        return vk::Format::eB10G11R11UfloatPack32; // the texels must still be 32-bits, so if no alpha channel then use more bits in other channels
         case TextureFormat::BGRA8:       return vk::Format::eB8G8R8A8Srgb;
         case TextureFormat::R8:          return vk::Format::eR8Unorm;
         case TextureFormat::R32F:        return vk::Format::eR32Sfloat;
         case TextureFormat::D32F:        return vk::Format::eD32Sfloat;
         case TextureFormat::D24S8:       return vk::Format::eD24UnormS8Uint;
         case TextureFormat::D32S8:       return vk::Format::eD32SfloatS8Uint;
         case TextureFormat::DXT1RGBA:    return vk::Format::eBc1RgbaUnormBlock;
         case TextureFormat::DXT1SRGBA:   return vk::Format::eBc1RgbaSrgbBlock;
         case TextureFormat::DXT3RGBA:    return vk::Format::eBc2UnormBlock;
         case TextureFormat::DXT3SRGBA:   return vk::Format::eBc2SrgbBlock;
         case TextureFormat::DXT5RGBA:    return vk::Format::eBc3UnormBlock;
         case TextureFormat::DXT5SRGBA:   return vk::Format::eBc3SrgbBlock;
         case TextureFormat::RGTC1R:      return vk::Format::eBc4UnormBlock;
         case TextureFormat::RGTC1SR:     return vk::Format::eBc4SnormBlock;
         case TextureFormat::RGTC2RG:     return vk::Format::eBc5UnormBlock;
         case TextureFormat::RGTC2SRG:    return vk::Format::eBc5SnormBlock;
         default:                         break;
      }
      PKZL_CORE_ASSERT(false, "Unsupported TextureFormat!");
      return vk::Format::eUndefined;
   }


   vk::Filter TextureFilterToVkFilter(const TextureFilter filter) {
      switch (filter) {
         case TextureFilter::Nearest:                return vk::Filter::eNearest;
         case TextureFilter::NearestMipmapNearest:   return vk::Filter::eNearest;
         case TextureFilter::NearestMipmapLinear:    return vk::Filter::eNearest;
         case TextureFilter::Linear:                 return vk::Filter::eLinear;
         case TextureFilter::LinearMipmapNearest:    return vk::Filter::eLinear;
         case TextureFilter::LinearMipmapLinear:     return vk::Filter::eLinear;
         default:                                    break;
      }
      PKZL_CORE_ASSERT(false, "Unsupported TextureFilter!");
      return vk::Filter::eLinear;
   }


   vk::SamplerMipmapMode TextureFilterToVkMipmapMode(const TextureFilter filter) {
      switch (filter) {
         case TextureFilter::Nearest:                return vk::SamplerMipmapMode::eNearest;
         case TextureFilter::NearestMipmapNearest:   return vk::SamplerMipmapMode::eNearest;
         case TextureFilter::NearestMipmapLinear:    return vk::SamplerMipmapMode::eLinear;
         case TextureFilter::Linear:                 return vk::SamplerMipmapMode::eLinear;
         case TextureFilter::LinearMipmapNearest:    return vk::SamplerMipmapMode::eNearest;
         case TextureFilter::LinearMipmapLinear:     return vk::SamplerMipmapMode::eLinear;
         default:                                    break;
      }
      PKZL_CORE_ASSERT(false, "Unsupported TextureFilter!");
      return vk::SamplerMipmapMode::eLinear;
   }


   vk::SamplerAddressMode TextureWrapToVkSamplerAddressMode(const TextureWrap wrap) {
      switch (wrap) {
         case TextureWrap::ClampToEdge:    return vk::SamplerAddressMode::eClampToEdge;
         case TextureWrap::ClampToBorder:  return vk::SamplerAddressMode::eClampToBorder;
         case TextureWrap::Repeat:         return vk::SamplerAddressMode::eRepeat;
         case TextureWrap::MirrorRepeat:   return vk::SamplerAddressMode::eMirroredRepeat;
         default:                          break;
      }
      PKZL_CORE_ASSERT(false, "Unsupported TextureWrap!");
      return vk::SamplerAddressMode::eClampToEdge;
   }


   VulkanTexture::~VulkanTexture() {
      DestroySampler();
      DestroyImage();
   }


   void VulkanTexture::Init(std::shared_ptr<VulkanDevice> device, const TextureSettings& settings, vk::ImageUsageFlags usage, vk::ImageAspectFlags aspect) {
      m_Device = device;
      m_Path = settings.path;

      if (m_Path.empty()) {
         uint32_t depth = CheckDepth(settings.depth);
         uint32_t layers = CheckLayers(settings.layers);
         CreateImage(TextureTypeToVkImageViewType(GetType()), settings.width, settings.height, layers * depth, settings.mipLevels, TextureFormatToVkFormat(settings.format), usage, aspect);
      } else {
         TextureLoader loader{ m_Path };
         if (!loader.IsLoaded()) {
            throw std::runtime_error{fmt::format("failed to load image '{}'", m_Path.string())};
         }
         uint32_t width = loader.GetWidth();
         uint32_t height = loader.GetHeight();
         uint32_t depth = CheckDepth(loader.GetDepth()); // loader will return 6 here for cubemaps.  But note that we define cubemaps has having depth 1
         uint32_t layers = CheckLayers(loader.GetLayers());
         TextureFormat format = loader.GetFormat();

         if (
            (
               (GetType() == TextureType::TextureCube) ||
               (GetType() == TextureType::TextureCubeArray)
            ) &&
            (loader.GetDepth() == 1)
         ) {
            // Loading a cubemap from a single 2d image.
            format = TextureFormat::RGBA16F;
            uint32_t width = loader.GetWidth();
            uint32_t height = loader.GetHeight();

            m_DataFormat = loader.GetFormat();
            if (!IsLinearColorSpace(settings.format)) {
               if (m_DataFormat == TextureFormat::RGBA8) {
                  m_DataFormat = TextureFormat::SRGBA8;
               } else if (m_DataFormat == TextureFormat::RGB8) {
                  m_DataFormat = TextureFormat::SRGB8;
               }
            }

            // guess whether the data is the 6-faces of a cube, or whether it's equirectangular
            // width is twice the height -> equirectangular (probably)
            // width is 4/3 the height -> 6 faces of a cube (probably)
            if (width / 2 == height) {
               width = height;
            } else {
               width /= 4;
               height = width;
            }

            uint32_t mipLevels = settings.mipLevels;
            if (mipLevels == 0) {
               mipLevels = std::max(CalculateMipmapLevels(width, height), loader.GetMIPLevels());
            }
            CreateImage(TextureTypeToVkImageViewType(GetType()), width, height, layers * depth, mipLevels, TextureFormatToVkFormat(format), usage | vk::ImageUsageFlagBits::eStorage, aspect);

            const auto [data, size] = loader.GetData(0, 0, 0);
            SetData(data, size);
         } else {
            // Depending on image file format, the loader may not be able to tell whether the image was in
            // sRGB colorspace.  Adjust format via hint in settings
            if (!IsLinearColorSpace(settings.format)) {
               if (format == TextureFormat::RGBA8) {
                  format = TextureFormat::SRGBA8;
               } else if (format == TextureFormat::RGB8) {
                  format = TextureFormat::SRGB8;
               }
            }

            uint32_t mipLevels = settings.mipLevels;
            if (mipLevels == 0) {
               mipLevels = std::max(CalculateMipmapLevels(width, height), loader.GetMIPLevels());
            }
            CreateImage(TextureTypeToVkImageViewType(GetType()), width, height, layers * depth, mipLevels, TextureFormatToVkFormat(format), usage, aspect);

              for (uint32_t layer = 0; layer < layers; ++layer) {

               // note: loader.GetDepth() is the actual number of slices in the data
               //       this->GetDepth() could be different (e.g. cubemaps will have loader.GetDepth() = 6, and this->GetDepth() = 1)
               for (uint32_t slice = 0; slice < loader.GetDepth(); ++slice) {
                  for (uint32_t mipLevel = 0; mipLevel < std::min(mipLevels, loader.GetMIPLevels()); ++mipLevel) {
                     const auto [data, size] = loader.GetData(layer, slice, mipLevel);
                     SetDataInternal(layer, slice, mipLevel, size, data);
                  }
               }
            }
            // here you have levels 0..min in transfer_dst_optimal,  and all the other ones undefined
            // put the rest of them into transfer_dst_optimal also
            uint32_t baseMipLevel = std::min(mipLevels, loader.GetMIPLevels()) - 1;
            if (baseMipLevel < GetMIPLevels() - 1) {
               m_Device->PipelineBarrier(
                  vk::PipelineStageFlagBits::eAllCommands,
                  vk::PipelineStageFlagBits::eTransfer,
                  m_Image->Barrier(vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal, baseMipLevel + 1, (GetMIPLevels() - baseMipLevel - 1), 0, 0)
               );
            }
            m_Image->GenerateMipmap(baseMipLevel);
         }
      }
      CreateSampler(settings);
   }


   uint32_t VulkanTexture::GetWidth() const {
      return m_Image->GetWidth();
   }


   uint32_t VulkanTexture::GetHeight() const {
      return m_Image->GetHeight();
   }


   uint32_t VulkanTexture::GetDepth() const {
      return m_Image->GetDepth();
   }


   uint32_t VulkanTexture::GetLayers() const {
      return m_Image->GetLayers();
   }


   uint32_t VulkanTexture::GetMIPLevels() const {
      return m_Image->GetMIPLevels();
   }


   TextureFormat VulkanTexture::GetFormat() const {
      return VkFormatToTextureFormat(m_Image->GetVkFormat());
   }


   void VulkanTexture::Commit(const uint32_t baseMipLevel) {
      if (baseMipLevel < GetMIPLevels()) {
         m_Device->PipelineBarrier(
            vk::PipelineStageFlagBits::eAllCommands,
            vk::PipelineStageFlagBits::eTransfer,
            m_Image->Barrier(vk::ImageLayout::eGeneral, vk::ImageLayout::eTransferDstOptimal, 0, 0, 0, 0)
         );
         m_Image->GenerateMipmap(baseMipLevel);
      } else {
         m_Device->PipelineBarrier(
            vk::PipelineStageFlagBits::eAllCommands,
            vk::PipelineStageFlagBits::eFragmentShader,
            m_Image->Barrier(vk::ImageLayout::eGeneral, vk::ImageLayout::eShaderReadOnlyOptimal, 0, 0, 0, 0)
         );
      }
   }


   bool VulkanTexture::operator==(const Texture& that) {
      return m_Image->GetVkImage() == static_cast<const VulkanTexture&>(that).m_Image->GetVkImage();
   }


   void VulkanTexture::CopyFrom(const Texture& srcTexture, const TextureCopySettings& settings) {
      if (GetType() != srcTexture.GetType()) {
         throw std::logic_error{fmt::format("Texture::CopyFrom() source and destination textures are not the same type!")};
      }
      if (GetFormat() != srcTexture.GetFormat()) {
         throw std::logic_error{fmt::format("Texture::CopyFrom() source and destination textures are not the same format!")};
      }
      if (settings.srcMipLevel >= srcTexture.GetMIPLevels()) {
         throw std::logic_error{fmt::format("Texture::CopyFrom() source texture does not have requested mip level!")};
      }
      if (settings.dstMipLevel >= GetMIPLevels()) {
         throw std::logic_error{fmt::format("Texture::CopyFrom() destination texture does not have requested mip level!")};
      }

      uint32_t layerCount = settings.layerCount == 0 ? srcTexture.GetLayers() : settings.layerCount;
      if (settings.srcLayer + layerCount > srcTexture.GetLayers()) {
         throw std::logic_error{fmt::format("Texture::CopyFrom() source texture does not have requested layer!")};
      }
      if (settings.dstLayer + layerCount > GetLayers()) {
         throw std::logic_error{fmt::format("Texture::CopyFrom() destination texture does not have requested layer!")};
      }

      uint32_t width = settings.width == 0 ? srcTexture.GetWidth() / (1 << settings.srcMipLevel) : settings.width;
      uint32_t height = settings.height == 0 ? srcTexture.GetHeight() / (1 << settings.srcMipLevel) : settings.height;

      if (width > GetWidth() / (1 << settings.dstMipLevel)) {
         throw std::logic_error{fmt::format("Texture::CopyFrom() requested width is larger than destination texture width!")};
      }
      if (height > GetHeight() / (1 << settings.dstMipLevel)) {
         throw std::logic_error{fmt::format("Texture::CopyFrom() requested height is larger than destination texture height!")};
      }

      vk::ImageCopy region = {
         {
            IsDepthFormat(srcTexture.GetFormat())? vk::ImageAspectFlagBits::eDepth : vk::ImageAspectFlagBits::eColor,
            settings.srcMipLevel,
            settings.srcLayer,
            layerCount
         } /*srcSubresource*/,
         {
            settings.srcX,
            settings.srcY,
            settings.srcZ
         } /*srcOffset*/,
         {
            IsDepthFormat(GetFormat()) ? vk::ImageAspectFlagBits::eDepth : vk::ImageAspectFlagBits::eColor,
            settings.dstMipLevel,
            settings.dstLayer,
            layerCount
         } /*dstSubresource*/,
         {
            settings.dstX,
            settings.dstY,
            settings.dstZ
         } /*dstOffset*/,
         {
            width,
            height,
            1  /*depth*/
         } /*extent*/
      };
      VulkanTexture& vulkanTexture = static_cast<VulkanTexture&>(const_cast<Texture&>(srcTexture));
      m_Image->CopyFromImage(vulkanTexture.GetImage(), region);
   }


   vk::Format VulkanTexture::GetVkFormat() const {
      return m_Image->GetVkFormat();
   }


   vk::Image VulkanTexture::GetVkImage() const {
      return m_Image->GetVkImage();
   }


   vk::ImageView VulkanTexture::GetVkImageView() const {
      return m_Image->GetVkImageView();
   }


   vk::ImageView VulkanTexture::GetVkImageView(const uint32_t mipLevel) const {
      return m_Image->GetVkImageView(mipLevel);
   }


   vk::Sampler VulkanTexture::GetVkSampler() const {
      return m_TextureSampler;
   }


   const Pikzel::VulkanImage& VulkanTexture::GetImage() const {
      PKZL_CORE_ASSERT(m_Image, "Attempted to access null image!");
      return *m_Image;
   }


   void VulkanTexture::SetDataInternal(const uint32_t layer, const uint32_t slice, const uint32_t mipLevel, const uint32_t size, const void* data) {
      VulkanBuffer stagingBuffer(m_Device, size, vk::BufferUsageFlagBits::eTransferSrc, vma::MemoryUsage::eCpuToGpu);
      stagingBuffer.CopyFromHost(0, size, data);

      uint32_t width = GetWidth() >> mipLevel;
      uint32_t height = GetHeight() >> mipLevel;
      uint32_t depth = 1;

      uint32_t layerInternal = layer;
      int32_t sliceInternal = slice;
      if (
         (GetType() == TextureType::TextureCube) ||
         (GetType() == TextureType::TextureCubeArray)
      ) {
         // loader puts cubemap faces into the "slice"
         // but vulkan treats cubemap faces as layers.
         layerInternal = (layer * 6) + slice;
         sliceInternal = 0;
      }

      vk::BufferImageCopy region = {
         0                                    /*bufferOffset*/,
         0                                    /*bufferRowLength*/,
         0                                    /*bufferImageHeight*/,
         vk::ImageSubresourceLayers {
            vk::ImageAspectFlagBits::eColor      /*aspectMask*/,
            mipLevel                             /*mipLevel*/,
            layerInternal                        /*baseArrayLayer*/,
            1                                    /*layerCount*/
         }                                    /*imageSubresource*/,
         {0, 0, sliceInternal}                /*imageOffset*/,
         {width, height, depth}               /*imageExtent*/
      };

      m_Image->CopyFromBuffer(stagingBuffer.m_Buffer, region);
   }


   void VulkanTexture::CreateImage(const vk::ImageViewType type, const uint32_t width, const uint32_t height, const uint32_t layers, const uint32_t mipLevels, const vk::Format format, vk::ImageUsageFlags usage, vk::ImageAspectFlags aspect) {
      m_Image = std::make_unique<VulkanImage>(
         m_Device,
         type,
         width,
         height,
         layers,
         mipLevels == 0? CalculateMipmapLevels(width, height) : mipLevels,
         vk::SampleCountFlagBits::e1,
         format,
         vk::ImageTiling::eOptimal,
         usage | vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled,
         vma::MemoryUsage::eGpuOnly
      );
      m_Image->CreateImageViews(format, aspect);
      if (usage & vk::ImageUsageFlagBits::eStorage) {
         m_Device->PipelineBarrier(
            vk::PipelineStageFlagBits::eAllCommands,
            vk::PipelineStageFlagBits::eAllCommands,
            m_Image->Barrier(vk::ImageLayout::eUndefined, vk::ImageLayout::eGeneral, 0, 0, 0, 0)
         );
      }

   }


   void VulkanTexture::DestroyImage() {
      m_Image.reset();
   }


   void VulkanTexture::CreateSampler(const TextureSettings& settings) {
      TextureFilter minFilter = settings.minFilter;
      TextureFilter magFilter = settings.magFilter;
      TextureWrap wrapU = settings.wrapU;
      TextureWrap wrapV = settings.wrapV;
      TextureWrap wrapW = settings.wrapW;

      if (minFilter == TextureFilter::Undefined) {
         minFilter = IsDepthFormat(GetFormat()) ? TextureFilter::Nearest : m_Image->GetMIPLevels() == 1 ? TextureFilter::Linear : TextureFilter::LinearMipmapLinear;
      }
      if (magFilter == TextureFilter::Undefined) {
         magFilter = IsDepthFormat(GetFormat()) ? TextureFilter::Nearest : TextureFilter::Linear;
      }

      if (wrapU == TextureWrap::Undefined) {
         wrapU = IsDepthFormat(GetFormat()) ? TextureWrap::ClampToEdge : TextureWrap::Repeat;
      }
      if (wrapV == TextureWrap::Undefined) {
         wrapV = IsDepthFormat(GetFormat()) ? TextureWrap::ClampToEdge : TextureWrap::Repeat;
      }
      if (wrapW == TextureWrap::Undefined) {
         wrapW = IsDepthFormat(GetFormat()) ? TextureWrap::ClampToEdge : TextureWrap::Repeat;
      }

      m_TextureSampler = m_Device->GetVkDevice().createSampler({
         {}                                                                       /*flags*/,
         TextureFilterToVkFilter(magFilter)                                       /*magFilter*/,
         TextureFilterToVkFilter(minFilter)                                       /*minFilter*/,
         TextureFilterToVkMipmapMode(minFilter)                                   /*mipmapMode*/,
         TextureWrapToVkSamplerAddressMode(wrapU)                                 /*addressModeU*/,
         TextureWrapToVkSamplerAddressMode(wrapV)                                 /*addressModeV*/,
         TextureWrapToVkSamplerAddressMode(wrapW)                                 /*addressModeW*/,
         0.0f                                                                     /*mipLodBias*/,
         m_Device->GetEnabledPhysicalDeviceFeatures<vk::PhysicalDeviceFeatures>().samplerAnisotropy  /*anisotropyEnable*/,
         16                                                                       /*maxAnisotropy*/,
         false                                                                    /*compareEnable*/,
         vk::CompareOp::eNever                                                    /*compareOp*/,
         0.0f                                                                     /*minLod*/,
         static_cast<float>(m_Image->GetMIPLevels())                              /*maxLod*/,
         vk::BorderColor::eFloatOpaqueBlack                                       /*borderColor*/,
         false                                                                    /*unnormalizedCoordinates*/
      });
   }


   void VulkanTexture::DestroySampler() {
      if (m_Device && m_TextureSampler) {
         m_Device->GetVkDevice().destroy(m_TextureSampler);
         m_TextureSampler = nullptr;
      }
   }


   VulkanTexture2D::VulkanTexture2D(std::shared_ptr<VulkanDevice> device, const TextureSettings& settings, vk::ImageUsageFlags usage, vk::ImageAspectFlags aspect) {
      Init(device, settings, usage, aspect);
   }


   TextureType VulkanTexture2D::GetType() const {
      return TextureType::Texture2D;
   }


   void VulkanTexture2D::SetData(const void* data, uint32_t size) {
      VulkanBuffer stagingBuffer(m_Device, size, vk::BufferUsageFlagBits::eTransferSrc, vma::MemoryUsage::eCpuToGpu);
      stagingBuffer.CopyFromHost(0, size, data);

      vk::BufferImageCopy region = {
         0                                     /*bufferOffset*/,
         0                                     /*bufferRowLength*/,
         0                                     /*bufferImageHeight*/,
         vk::ImageSubresourceLayers {          
            vk::ImageAspectFlagBits::eColor       /*aspectMask*/,
            0                                     /*mipLevel*/,
            0                                     /*baseArrayLayer*/,
            GetLayers()                           /*layerCount*/
         }                                     /*imageSubresource*/,
         {0, 0, 0}                             /*imageOffset*/,
         {GetWidth(), GetHeight(), GetDepth()} /*imageExtent*/
      };

      m_Image->CopyFromBuffer(stagingBuffer.m_Buffer, region);
      m_Image->GenerateMipmap(0); // equivalent to Commit()
   }


   uint32_t VulkanTexture2D::CheckDepth(uint32_t depth) const {
      PKZL_CORE_ASSERT(depth == 1, "Depth for 2D texture should be 1!");
      return 1;
   }


   uint32_t VulkanTexture2D::CheckLayers(uint32_t layers) const {
      PKZL_CORE_ASSERT(layers == 1, "Layers for 2D texture should be 1!");
      return 1;
   }

   VulkanTexture2DArray::VulkanTexture2DArray(std::shared_ptr<VulkanDevice> device, const TextureSettings& settings, vk::ImageUsageFlags usage, vk::ImageAspectFlags aspect) {
      Init(device, settings, usage, aspect);
   }


   TextureType VulkanTexture2DArray::GetType() const {
      return TextureType::Texture2DArray;
   }


   void VulkanTexture2DArray::SetData(const void* data, const uint32_t size) {
      VulkanBuffer stagingBuffer(m_Device, size, vk::BufferUsageFlagBits::eTransferSrc, vma::MemoryUsage::eCpuToGpu);
      stagingBuffer.CopyFromHost(0, size, data);
      vk::BufferImageCopy region = {
         0                                     /*bufferOffset*/,
         0                                     /*bufferRowLength*/,
         0                                     /*bufferImageHeight*/,
         vk::ImageSubresourceLayers {
            vk::ImageAspectFlagBits::eColor       /*aspectMask*/,
            0                                     /*mipLevel*/,
            0                                     /*baseArrayLayer*/,
            GetLayers()                           /*layerCount*/
         }                                     /*imageSubresource*/,
         {0, 0, 0}                             /*imageOffset*/,
         {GetWidth(), GetHeight(), GetDepth()} /*imageExtent*/
      };
      m_Image->CopyFromBuffer(stagingBuffer.m_Buffer, region);
      m_Image->GenerateMipmap(0); // equivalent to Commit()
   }


   uint32_t VulkanTexture2DArray::CheckDepth(uint32_t depth) const {
      PKZL_CORE_ASSERT(depth == 1, "Depth for 2D texture array should be 1!");
      return 1;
   }


   uint32_t VulkanTexture2DArray::CheckLayers(uint32_t layers) const {
      return layers;
   }


   VulkanTextureCube::VulkanTextureCube(std::shared_ptr<VulkanDevice> device, const TextureSettings& settings, vk::ImageUsageFlags usage, vk::ImageAspectFlags aspect) {
      Init(device, settings, usage, aspect);
   }


   TextureType VulkanTextureCube::GetType() const {
      return TextureType::TextureCube;
   }


   void VulkanTextureCube::SetData(const void* data, const uint32_t size) {
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

      std::unique_ptr<VulkanTexture2D> tex2d = std::make_unique<VulkanTexture2D>(m_Device, TextureSettings{.width = width, .height = height, .format = m_DataFormat});
      tex2d->SetData(data, size);

      std::unique_ptr<ComputeContext> compute = std::make_unique<VulkanComputeContext>(m_Device);
      std::unique_ptr<Pipeline> pipeline = compute->CreatePipeline({
         .shaders = {
            { Pikzel::ShaderType::Compute, shader }
         }
      });
      compute->Begin();
      compute->Bind(*pipeline);
      compute->Bind("uTexture"_hs, *tex2d);
      compute->Bind("outCubeMap"_hs, *this);
      compute->Dispatch(GetWidth() / 32, GetHeight() / 32, 6);
      compute->End();
      Commit(0);
   }


   uint32_t VulkanTextureCube::CheckDepth(uint32_t depth) const {
      PKZL_CORE_ASSERT(depth == 1 || depth == 6, "Depth for cube texture should be 1!"); // 6 is also acceptable, but we ignore that and return 1 anyway
      return 1;
   }


   uint32_t VulkanTextureCube::CheckLayers(uint32_t layers) const {
      PKZL_CORE_ASSERT(layers == 1, "Layers for cube texture should be 1!");
      return 1;
   }

   VulkanTextureCubeArray::VulkanTextureCubeArray(std::shared_ptr<VulkanDevice> device, const TextureSettings& settings, vk::ImageUsageFlags usage, vk::ImageAspectFlags aspect) {
      Init(device, settings, usage, aspect);
   }


   TextureType VulkanTextureCubeArray::GetType() const {
      return TextureType::TextureCubeArray;
   }


   void VulkanTextureCubeArray::SetData(const void* data, const uint32_t size) {
      PKZL_NOT_IMPLEMENTED;
   }


   uint32_t VulkanTextureCubeArray::CheckDepth(uint32_t depth) const {
      PKZL_CORE_ASSERT(depth == 1 || depth == 6, "Depth for cube texture array should be 1!"); // 6 is also acceptable, but we ignore that and return 1 anyway
      return 1;
   }


   uint32_t VulkanTextureCubeArray::CheckLayers(uint32_t layers) const {
      return layers;
   }

}
