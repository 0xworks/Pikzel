#include "VulkanTexture.h"

#include "VulkanBuffer.h"
#include "VulkanComputeContext.h"
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
         case TextureFormat::RG16F:    return vk::Format::eR16G16Sfloat;
         case TextureFormat::RGB16F:   return vk::Format::eR16G16B16Sfloat;
         case TextureFormat::RGBA16F:  return vk::Format::eR16G16B16A16Sfloat;
         case TextureFormat::RG32F:    return vk::Format::eR32G32Sfloat;
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


   vk::Filter TextureFilterToVkFilter(const TextureFilter filter) {
      switch (filter) {
         case TextureFilter::Nearest:                return vk::Filter::eNearest;
         case TextureFilter::NearestMipmapNearest:   return vk::Filter::eNearest;
         case TextureFilter::NearestMipmapLinear:    return vk::Filter::eNearest;
         case TextureFilter::Linear:                 return vk::Filter::eLinear;
         case TextureFilter::LinearMipmapNearest:    return vk::Filter::eLinear;
         case TextureFilter::LinearMipmapLinear:     return vk::Filter::eLinear;
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
      }
      PKZL_CORE_ASSERT(false, "Unsupported TextureWrap!");
      return vk::SamplerAddressMode::eClampToEdge;
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
      } else if (channels == 1) {
         *format = isHDR ? TextureFormat::R32F : TextureFormat::R8;
      } else {
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


   uint32_t VulkanTexture::GetMIPLevels() const {
      return m_Image->GetMIPLevels();
   }


   TextureFormat VulkanTexture::GetFormat() const {
      return VkFormatToTextureFormat(m_Image->GetVkFormat());
   }


   void VulkanTexture::Commit(const bool generateMipmap) {
      if (generateMipmap) {
         m_Device->PipelineBarrier(
            vk::PipelineStageFlagBits::eAllCommands,
            vk::PipelineStageFlagBits::eTransfer,
            m_Image->Barrier(vk::ImageLayout::eGeneral, vk::ImageLayout::eTransferDstOptimal, 0, 0, 0, 0)
         );
         m_Image->GenerateMipmap();
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
         throw std::logic_error {fmt::format("Texture::CopyFrom() source and destination textures are not the same type!")};
      }
      if (GetFormat() != srcTexture.GetFormat()) {
         throw std::logic_error {fmt::format("Texture::CopyFrom() source and destination textures are not the same format!")};
      }
      if (settings.srcMipLevel >= srcTexture.GetMIPLevels()) {
         throw std::logic_error {fmt::format("Texture::CopyFrom() source texture does not have requested mip level!")};
      }
      if (settings.dstMipLevel >= GetMIPLevels()) {
         throw std::logic_error {fmt::format("Texture::CopyFrom() destination texture does not have requested mip level!")};
      }

      uint32_t layerCount = settings.layerCount == 0 ? srcTexture.GetLayers() : settings.layerCount;
      if (settings.srcLayer + layerCount > srcTexture.GetLayers()) {
         throw std::logic_error {fmt::format("Texture::CopyFrom() source texture does not have requested layer!")};
      }
      if (settings.dstLayer + layerCount > GetLayers()) {
         throw std::logic_error {fmt::format("Texture::CopyFrom() destination texture does not have requested layer!")};
      }

      uint32_t width = settings.width == 0 ? srcTexture.GetWidth() / (1 << settings.srcMipLevel) : settings.width;
      uint32_t height = settings.height == 0 ? srcTexture.GetHeight() / (1 << settings.srcMipLevel) : settings.height;

      if (width > GetWidth() / (1 << settings.dstMipLevel)) {
         throw std::logic_error {fmt::format("Texture::CopyFrom() requested width is larger than destination texture width!")};
      }
      if (height > GetHeight() / (1 << settings.dstMipLevel)) {
         throw std::logic_error {fmt::format("Texture::CopyFrom() requested height is larger than destination texture height!")};
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
         {}                                                              /*flags*/,
         TextureFilterToVkFilter(magFilter)                              /*magFilter*/,
         TextureFilterToVkFilter(minFilter)                              /*minFilter*/,
         TextureFilterToVkMipmapMode(minFilter)                          /*mipmapMode*/,
         TextureWrapToVkSamplerAddressMode(wrapU)                        /*addressModeU*/,
         TextureWrapToVkSamplerAddressMode(wrapV)                        /*addressModeV*/,
         TextureWrapToVkSamplerAddressMode(wrapW)                        /*addressModeW*/,
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


   void VulkanTexture::DestroySampler() {
      if (m_Device && m_TextureSampler) {
         m_Device->GetVkDevice().destroy(m_TextureSampler);
         m_TextureSampler = nullptr;
      }
   }


   VulkanTexture2D::VulkanTexture2D(std::shared_ptr<VulkanDevice> device, const TextureSettings& settings, vk::ImageUsageFlags usage, vk::ImageAspectFlags aspect)
   : m_Path {settings.path}
   {
      m_Device = device;
      if (m_Path.empty()) {
         CreateImage(vk::ImageViewType::e2D, settings.width, settings.height, 1, settings.mipLevels, TextureFormatToVkFormat(settings.format), usage, aspect);
      } else {
         uint32_t width;
         uint32_t height;
         TextureFormat format;
         stbi_uc* data = STBILoad(m_Path, !IsLinearColorSpace(settings.format), &width, &height, &format);
         CreateImage(vk::ImageViewType::e2D, width, height, 1, settings.mipLevels, TextureFormatToVkFormat(format), usage, aspect);
         PKZL_CORE_ASSERT(BPP(format) != 3, "VulkanTexture2D format cannot be 24-bits per texel");
         SetData(data, width * height * BPP(format));
         stbi_image_free(data);
      }
      CreateSampler(settings);
   }


   TextureType VulkanTexture2D::GetType() const {
      return TextureType::Texture2D;
   }


   void VulkanTexture2D::SetData(void* data, uint32_t size) {
      VulkanBuffer stagingBuffer(m_Device, size, vk::BufferUsageFlagBits::eTransferSrc, vma::MemoryUsage::eCpuToGpu);
      stagingBuffer.CopyFromHost(0, size, data);
      m_Device->PipelineBarrier(
         vk::PipelineStageFlagBits::eTopOfPipe,
         vk::PipelineStageFlagBits::eTransfer,
         m_Image->Barrier(vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal, 0, 0, 0, 0)
      );
      m_Image->CopyFromBuffer(stagingBuffer.m_Buffer);
      m_Image->GenerateMipmap();
   }


   VulkanTexture2DArray::VulkanTexture2DArray(std::shared_ptr<VulkanDevice> device, const TextureSettings& settings, vk::ImageUsageFlags usage, vk::ImageAspectFlags aspect) {
      m_Device = device;
      CreateImage(vk::ImageViewType::e2DArray, settings.width, settings.height, settings.layers, settings.mipLevels, TextureFormatToVkFormat(settings.format), usage, aspect);
      CreateSampler(settings);
   }


   TextureType VulkanTexture2DArray::GetType() const {
      return TextureType::Texture2DArray;
   }


   void VulkanTexture2DArray::SetData(void* data, const uint32_t size) {
      VulkanBuffer stagingBuffer(m_Device, size, vk::BufferUsageFlagBits::eTransferSrc, vma::MemoryUsage::eCpuToGpu);
      stagingBuffer.CopyFromHost(0, size, data);
      m_Device->PipelineBarrier(
         vk::PipelineStageFlagBits::eTopOfPipe,
         vk::PipelineStageFlagBits::eTransfer,
         m_Image->Barrier(vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal, 0, 0, 0, 0)
      );
      m_Image->CopyFromBuffer(stagingBuffer.m_Buffer);
      m_Image->GenerateMipmap();
   }


   VulkanTextureCube::VulkanTextureCube(std::shared_ptr<VulkanDevice> device, const TextureSettings& settings, vk::ImageUsageFlags usage, vk::ImageAspectFlags aspect)
   : m_Path {settings.path}
   {
      m_Device = device;
      if (m_Path.empty()) {
         CreateImage(vk::ImageViewType::eCube, settings.width, settings.height, 1, settings.mipLevels, TextureFormatToVkFormat(settings.format), usage, aspect);
      } else {
         uint32_t width;
         uint32_t height;
         stbi_uc* data = STBILoad(m_Path, !IsLinearColorSpace(settings.format), &width, &height, &m_DataFormat);

         // guess whether the data is the 6-faces of a cube, or whether it's equirectangular
         // width is twice the height -> equirectangular (probably)
         // width is 4/3 the height -> 6 faces of a cube (probably)
         uint32_t size = 0;
         if (width / 2 == height) {
            size = height;
         } else {
            size = width / 4;
         }
         CreateImage(vk::ImageViewType::eCube, size, size, 1, settings.mipLevels, TextureFormatToVkFormat(TextureFormat::RGBA16F), usage | vk::ImageUsageFlagBits::eStorage, aspect);
         PKZL_CORE_ASSERT(BPP(m_DataFormat) != 3, "VulkanTextureCube format cannot be 24-bits per texel");
         SetData(data, width * height * BPP(m_DataFormat));
         stbi_image_free(data);
      }
      CreateSampler(settings);
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

      std::unique_ptr<VulkanTexture2D> tex2d = std::make_unique<VulkanTexture2D>(m_Device, TextureSettings{.width = width, .height = height, .format = m_DataFormat, .mipLevels = 1});
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
      Commit();
   }


   VulkanTextureCubeArray::VulkanTextureCubeArray(std::shared_ptr<VulkanDevice> device, const TextureSettings& settings, vk::ImageUsageFlags usage, vk::ImageAspectFlags aspect) {
      m_Device = device;
      CreateImage(vk::ImageViewType::eCubeArray, settings.width, settings.height, settings.layers, settings.mipLevels, TextureFormatToVkFormat(settings.format), usage, aspect);
      CreateSampler(settings);
   }


   TextureType VulkanTextureCubeArray::GetType() const {
      return TextureType::TextureCubeArray;
   }


   void VulkanTextureCubeArray::SetData(void* data, const uint32_t size) {
      PKZL_NOT_IMPLEMENTED;
   }

}
