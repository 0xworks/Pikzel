#include "VulkanTexture.h"

#include "VulkanBuffer.h"
#include "VulkanPipeline.h"

#include <stb_image.h>

namespace Pikzel {

   static TextureFormat VkFormatToTextureFormat(const vk::Format format) {
      switch (format) {
         case vk::Format::eR8G8B8Unorm: return TextureFormat::RGB8;
         case vk::Format::eR8G8B8A8Unorm: return TextureFormat::RGBA8;
         case vk::Format::eR32G32B32Sfloat: return TextureFormat::RGB32F;
         case vk::Format::eR32G32B32A32Sfloat: return TextureFormat::RGBA32F;
         case vk::Format::eB8G8R8A8Unorm: return TextureFormat::BGRA8;

      }
      PKZL_CORE_ASSERT(false, "Unknown TextureFormat!");
      return TextureFormat::Undefined;
   }


   static vk::Format TextureFormatToVkFormat(const TextureFormat format) {
      switch (format) {
         case TextureFormat::RGB8: return vk::Format::eR8G8B8Unorm;
         case TextureFormat::RGBA8: return vk::Format::eR8G8B8A8Unorm;
         case TextureFormat::RGB32F: return vk::Format::eR32G32B32Sfloat;
         case TextureFormat::RGBA32F: return vk::Format::eR32G32B32A32Sfloat;
         case TextureFormat::BGRA8: return vk::Format::eB8G8R8A8Unorm;
      }
      PKZL_CORE_ASSERT(false, "Unknown TextureFormat!");
      return vk::Format::eUndefined;
   }


   static stbi_uc* STBILoad(const std::filesystem::path& path, uint32_t& width, uint32_t& height, TextureFormat& format) {
      int iWidth;
      int iHeight;
      int channels;
      stbi_set_flip_vertically_on_load(1);
      stbi_uc* data = nullptr;
      bool isHDR = stbi_is_hdr(path.string().c_str());
      if (isHDR) {
         data = reinterpret_cast<stbi_uc*>(stbi_loadf(path.string().c_str(), &iWidth, &iHeight, &channels, STBI_rgb_alpha));
      } else {
         data = stbi_load(path.string().c_str(), &iWidth, &iHeight, &channels, STBI_rgb_alpha);
      }
      if (!data) {
         throw std::runtime_error {fmt::format("failed to load image '{0}'", path.string())};
      }
      width = static_cast<uint32_t>(iWidth);
      height = static_cast<uint32_t>(iHeight);

      if (channels == 4) {
         format = isHDR ? TextureFormat::RGBA32F : TextureFormat::RGBA8;
      } else if (channels == 3) {
         format = isHDR ? TextureFormat::RGBA32F : TextureFormat::RGBA8; // we forced in an alpha channel, above.
      } else {
         throw std::runtime_error {fmt::format("'{0}': Image format not supported!", path.string())};
      }

      return data;
   }


   VulkanTexture2D::VulkanTexture2D(std::shared_ptr<VulkanDevice> device, const uint32_t width, const uint32_t height, const TextureFormat format /*= TextureFormat::RGB8*/, const uint32_t mipLevels /*=1*/)
   : m_Device {device} 
   {
      CreateImage(width, height, TextureFormatToVkFormat(format), mipLevels);
      CreateSampler();
   }


   VulkanTexture2D::VulkanTexture2D(std::shared_ptr<VulkanDevice> device, const std::filesystem::path& path)
   : m_Path {path}
   , m_Device {device}
   {
      uint32_t width;
      uint32_t height;
      TextureFormat format;
      stbi_uc* data = STBILoad(path, width, height, format);

      CreateImage(width, height, TextureFormatToVkFormat(format), CalculateMipMapLevels(width, height));
      CreateSampler();
      SetData(data, width * height * BPP(format));

      stbi_image_free(data);
   }


   VulkanTexture2D::~VulkanTexture2D() {
      DestroySampler();
      DestroyImage();
   }


   TextureFormat VulkanTexture2D::GetFormat() const {
      return VkFormatToTextureFormat(m_Image->GetVkFormat());
   }


   uint32_t VulkanTexture2D::GetWidth() const {
      return m_Image->GetVkExtent().width;
   }


   uint32_t VulkanTexture2D::GetHeight() const {
      return m_Image->GetVkExtent().height;
   }


   void VulkanTexture2D::SetData(void* data, uint32_t size) {
      VulkanBuffer stagingBuffer(m_Device, size, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
      stagingBuffer.CopyFromHost(0, size, data);
      m_Image->TransitionImageLayout(vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal);
      m_Image->CopyFromBuffer(stagingBuffer.m_Buffer);
      m_Image->GenerateMIPMaps();
   }


   bool VulkanTexture2D::operator==(const Texture2D& that) {
      return m_Image->GetVkImage() == static_cast<const VulkanTexture2D&>(that).m_Image->GetVkImage();
   }


   vk::Sampler VulkanTexture2D::GetVkSampler() const {
      return m_TextureSampler;
   }


   vk::ImageView VulkanTexture2D::GetVkImageView() const {
      return m_Image->GetVkImageView();
   }


   vk::Format VulkanTexture2D::GetVkFormat() const {
      return m_Image->GetVkFormat();
   }


   void VulkanTexture2D::CreateImage(const uint32_t width, const uint32_t height, const vk::Format format, const uint32_t mipLevels) {
      m_Image = std::make_unique<VulkanImage>(
         m_Device,
         width,
         height,
         mipLevels,
         vk::SampleCountFlagBits::e1,
         format,
         vk::ImageTiling::eOptimal,
         vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eColorAttachment,
         vk::MemoryPropertyFlagBits::eDeviceLocal,
         vk::ImageCreateFlags {}
      );
      m_Image->CreateImageView(format, vk::ImageAspectFlagBits::eColor);
   }


   void VulkanTexture2D::DestroyImage() {
      m_Image.reset();
   }


   void VulkanTexture2D::CreateSampler() {
      m_TextureSampler = m_Device->GetVkDevice().createSampler({
         {}                                           /*flags*/,
         vk::Filter::eLinear                          /*magFilter*/,
         vk::Filter::eLinear                          /*minFilter*/,
         vk::SamplerMipmapMode::eLinear               /*mipmapMode*/,
         vk::SamplerAddressMode::eRepeat              /*addressModeU*/,
         vk::SamplerAddressMode::eRepeat              /*addressModeV*/,
         vk::SamplerAddressMode::eRepeat              /*addressModeW*/,
         0.0f                                         /*mipLodBias*/,
         true                                         /*anisotropyEnable*/,
         16                                           /*maxAnisotropy*/,
         false                                        /*compareEnable*/,
         vk::CompareOp::eNever                       /*compareOp*/,
         0.0f                                         /*minLod*/,
         static_cast<float>(m_Image->GetMIPLevels())  /*maxLod*/,
         vk::BorderColor::eFloatOpaqueBlack           /*borderColor*/,
         false                                        /*unnormalizedCoordinates*/
      });
   }


   void VulkanTexture2D::DestroySampler() {
      if (m_Device && m_TextureSampler) {
         m_Device->GetVkDevice().destroy(m_TextureSampler);
         m_TextureSampler = nullptr;
      }
   }


   VulkanTextureCube::VulkanTextureCube(std::shared_ptr<VulkanDevice> device, const uint32_t size, const TextureFormat format /*= TextureFormat::RGB8*/)
   : m_Device {device} {
      CreateImage(size, TextureFormatToVkFormat(format));
      CreateSampler();
   }


   VulkanTextureCube::VulkanTextureCube(std::shared_ptr<VulkanDevice> device, const std::filesystem::path& path)
   : m_Path {path}
   , m_Device {device}
   {
      uint32_t width;
      uint32_t height;
      stbi_uc* data = STBILoad(path, width, height, m_DataFormat);

      // guess whether the data is the 6-faces of a cube, or whether it's equirectangular
      // width is twice the height -> equirectangular (probably)
      // width is 4/3 the height -> 6 faces of a cube (probably)
      uint32_t size = 0;
      if (width / 2 == height) {
         size = height;
      } else {
         size = width / 4;
      }
      CreateImage(size, TextureFormatToVkFormat(TextureFormat::RGBA8));
      CreateSampler();
      SetData(data, width * height * BPP(m_DataFormat));

      stbi_image_free(data);
   }


   VulkanTextureCube::~VulkanTextureCube() {
      DestroySampler();
      DestroyImage();
   }


   TextureFormat VulkanTextureCube::GetFormat() const {
      return VkFormatToTextureFormat(m_Image->GetVkFormat());
   }


   uint32_t VulkanTextureCube::GetWidth() const {
      return m_Image->GetVkExtent().width;
   }


   uint32_t VulkanTextureCube::GetHeight() const {
      return m_Image->GetVkExtent().height;
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

      std::unique_ptr<VulkanTexture2D> tex2d = std::make_unique<VulkanTexture2D>(m_Device, width, height, m_DataFormat);
      tex2d->SetData(data, size);
      int tonemap = 0;
      float gamma = 1.0f;
      if (
         (tex2d->GetFormat() == TextureFormat::RGB32F) ||
         (tex2d->GetFormat() == TextureFormat::RGBA32F)
      ) {
         tonemap = 1;
         gamma = 2.2;
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
            texResource.Count                                        /*descriptorCount*/,
            texResource.Type                                         /*descriptorType*/,
            &texImageDescriptor                                      /*pImageInfo*/,
            nullptr                                                  /*pBufferInfo*/,
            nullptr                                                  /*pTexelBufferView*/
         },
         vk::WriteDescriptorSet {
            pipeline->GetVkDescriptorSet(cubeResource.DescriptorSet)  /*dstSet*/,
            cubeResource.Binding                                      /*dstBinding*/,
            0                                                         /*dstArrayElement*/,
            cubeResource.Count                                        /*descriptorCount*/,
            cubeResource.Type                                         /*descriptorType*/,
            &cubeImageDescriptor                                      /*pImageInfo*/,
            nullptr                                                   /*pBufferInfo*/,
            nullptr                                                   /*pTexelBufferView*/
         }
      };

      m_Device->GetVkDevice().updateDescriptorSets(writeDescriptors, nullptr);

      m_Device->SubmitSingleTimeCommands(m_Device->GetComputeQueue(), [this, &pipeline, &tex2d, tonemap, gamma] (vk::CommandBuffer cmd) {
         cmd.bindPipeline(vk::PipelineBindPoint::eCompute, pipeline->GetVkPipelineCompute());
         pipeline->BindDescriptorSets(cmd, vk::Fence {});

         const VulkanPushConstant& tonemapC = pipeline->GetPushConstant("constants.tonemap"_hs);
         PKZL_CORE_ASSERT(tonemapC.Type == DataType::Int, "Push constant '{0}' type mismatch.  Was {1}, expected int!", tonemapC.Name, DataTypeToString(tonemapC.Type));
         cmd.pushConstants<int>(pipeline->GetVkPipelineLayout(), tonemapC.ShaderStages, tonemapC.Offset, tonemap);

         const VulkanPushConstant& gammaC = pipeline->GetPushConstant("constants.gamma"_hs);
         PKZL_CORE_ASSERT(gammaC.Type == DataType::Float, "Push constant '{0}' type mismatch.  Was {1}, expected float!", gammaC.Name, DataTypeToString(gammaC.Type));
         cmd.pushConstants<float>(pipeline->GetVkPipelineLayout(), gammaC.ShaderStages, gammaC.Offset, gamma);

         cmd.dispatch(GetWidth() / 32, GetHeight() / 32, 6);
      });
      pipeline->UnbindDescriptorSets();

      m_Image->TransitionImageLayout(vk::ImageLayout::eGeneral, vk::ImageLayout::eTransferDstOptimal);
      m_Image->GenerateMIPMaps();
   }


   vk::Sampler VulkanTextureCube::GetVkSampler() const {
      return m_TextureSampler;
   }


   vk::ImageView VulkanTextureCube::GetVkImageView() const {
      return m_Image->GetVkImageView();
   }


   vk::Format VulkanTextureCube::GetVkFormat() const {
      return m_Image->GetVkFormat();
   }


   void VulkanTextureCube::CreateImage(const uint32_t size, const vk::Format format) {
      uint32_t mipLevels = CalculateMipMapLevels(size, size);
      m_Image = std::make_unique<VulkanImage>(
         m_Device,
         size,
         size,
         mipLevels,
         vk::SampleCountFlagBits::e1,
         format,
         vk::ImageTiling::eOptimal,
         vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eStorage | vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eColorAttachment,
         vk::MemoryPropertyFlagBits::eDeviceLocal,
         vk::ImageCreateFlagBits::eCubeCompatible
      );
      m_Image->CreateImageView(format, vk::ImageAspectFlagBits::eColor);
   }


   void VulkanTextureCube::DestroyImage() {
      m_Image.reset();
   }


   void VulkanTextureCube::CreateSampler() {
      m_TextureSampler = m_Device->GetVkDevice().createSampler({
         {}                                           /*flags*/,
         vk::Filter::eLinear                          /*magFilter*/,
         vk::Filter::eLinear                          /*minFilter*/,
         vk::SamplerMipmapMode::eLinear               /*mipmapMode*/,
         vk::SamplerAddressMode::eClampToEdge         /*addressModeU*/,
         vk::SamplerAddressMode::eClampToEdge         /*addressModeV*/,
         vk::SamplerAddressMode::eClampToEdge         /*addressModeW*/,
         0.0f                                         /*mipLodBias*/,
         true                                         /*anisotropyEnable*/,
         16                                           /*maxAnisotropy*/,
         false                                        /*compareEnable*/,
         vk::CompareOp::eNever                        /*compareOp*/,
         0.0f                                         /*minLod*/,
         static_cast<float>(m_Image->GetMIPLevels())  /*maxLod*/,
         vk::BorderColor::eFloatOpaqueBlack           /*borderColor*/,
         false                                        /*unnormalizedCoordinates*/
      });
   }


   void VulkanTextureCube::DestroySampler() {
      if (m_Device && m_TextureSampler) {
         m_Device->GetVkDevice().destroy(m_TextureSampler);
         m_TextureSampler = nullptr;
      }
   }


   bool VulkanTextureCube::operator==(const TextureCube& that) {
      return m_Image->GetVkImage() == static_cast<const VulkanTextureCube&>(that).m_Image->GetVkImage();
   }

}
