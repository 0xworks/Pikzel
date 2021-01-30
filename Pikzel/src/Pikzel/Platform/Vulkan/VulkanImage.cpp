#include "VulkanImage.h"
#include "VulkanUtility.h"

namespace Pikzel {

   VulkanImage::VulkanImage(std::shared_ptr<VulkanDevice> device, const vk::ImageViewType type, const uint32_t width, const uint32_t height, const uint32_t layers, const uint32_t mipLevels, vk::SampleCountFlagBits numSamples, const vk::Format format, const vk::ImageTiling tiling, const vk::ImageUsageFlags usage, const vma::MemoryUsage memoryUsage)
   : m_Device {device}
   , m_Type {type}
   , m_Format {format}
   , m_Width {width}
   , m_Height {height}
   , m_Depth {1}
   , m_MIPLevels {mipLevels}
   {
      vk::ImageType imageType = vk::ImageType::e2D;
      switch (type) {
         case vk::ImageViewType::e1D:
         case vk::ImageViewType::e1DArray:
            PKZL_NOT_IMPLEMENTED;
            break;
         case vk::ImageViewType::e2D:
         case vk::ImageViewType::e2DArray:
         case vk::ImageViewType::eCube:
         case vk::ImageViewType::eCubeArray:
            imageType = vk::ImageType::e2D;
            break;
         case vk::ImageViewType::e3D:
            imageType = vk::ImageType::e3D;
            PKZL_NOT_IMPLEMENTED;
            break;
      }

      vk::ImageCreateFlags flags = {};
      switch (type) {
         case vk::ImageViewType::e1D:
         case vk::ImageViewType::e1DArray:
         case vk::ImageViewType::e2D:
         case vk::ImageViewType::e3D:
            flags = {};
            break;
         case vk::ImageViewType::e2DArray:
            flags = vk::ImageCreateFlagBits::e2DArrayCompatible;
            break;
         case vk::ImageViewType::eCube:
         case vk::ImageViewType::eCubeArray:
            flags = vk::ImageCreateFlagBits::eCubeCompatible;
            break;
      }

      switch (type) {
         case vk::ImageViewType::e1D:
         case vk::ImageViewType::e1DArray:
         case vk::ImageViewType::e2D:
         case vk::ImageViewType::e2DArray:
         case vk::ImageViewType::e3D:
            m_Layers = layers;
            break;
         case vk::ImageViewType::eCube:
         case vk::ImageViewType::eCubeArray:
            m_Layers = layers * 6;
            break;
      }

      vk::ImageCreateInfo imageInfo = {
         flags                         /*flags*/,
         imageType                     /*imageType*/,
         format                        /*format*/,
         {m_Width, m_Height, m_Depth}  /*extent*/,
         mipLevels                     /*mipLevels*/,
         m_Layers                      /*arrayLayers*/,
         numSamples                    /*samples*/,
         tiling                        /*tiling*/,
         usage                         /*usage*/,
         vk::SharingMode::eExclusive   /*sharingMode*/,
         0                             /*queueFamilyIndexCount*/,
         nullptr                       /*pQueueFamilyIndices*/,
         vk::ImageLayout::eUndefined   /*initialLayout*/
      };

      vma::AllocationCreateInfo allocInfo = {};
      allocInfo.usage = memoryUsage;

      auto [image, allocation] = VulkanMemoryAllocator::Get().createImage(imageInfo, allocInfo);
      m_Image = image;
      m_Allocation = allocation;
   }


   VulkanImage::VulkanImage(std::shared_ptr<VulkanDevice> device, const vk::Image& image, vk::Format format, vk::Extent2D extent)
      : m_Device {device}
      , m_Type {vk::ImageViewType::e2D}
      , m_Image {image}
      , m_Format {format}
      , m_Width {extent.width}
      , m_Height {extent.height}
      , m_Depth {1}
      , m_MIPLevels {1}
      , m_Layers {1}
   {}


   VulkanImage::~VulkanImage() {
      DestroyImageViews();
      if (m_Device && m_Image && m_Allocation) {
         // Only destroy the image if it has an allocation.
         // I.e. we allocated the image, so we destroy it.
         // As opposed to images that were created (and are destroyed) by
         // the swap chain.
         VulkanMemoryAllocator::Get().destroyImage(m_Image, m_Allocation);
         m_Image = nullptr;
         m_Allocation = nullptr;
      }
   }


   vk::Image VulkanImage::GetVkImage() const {
      return m_Image;
   }


   vk::Format VulkanImage::GetVkFormat() const {
      return m_Format;
   }


   uint32_t VulkanImage::GetWidth() const {
      return m_Width;
   }


   uint32_t VulkanImage::GetHeight() const {
      return m_Height;
   }


   uint32_t VulkanImage::GetDepth() const {
      return m_Depth;
   }


   uint32_t VulkanImage::GetMIPLevels() const {
      return m_MIPLevels;
   }


   uint32_t VulkanImage::GetLayers() const {
      return m_Layers;
   }


   void VulkanImage::CreateImageViews(const vk::Format format, const vk::ImageAspectFlags imageAspect) {
      vk::ImageViewCreateInfo ci = {
         {}            /*flags*/,
         m_Image       /*image*/,
         m_Type        /*viewType*/,
         format        /*format*/,
         {}            /*components*/,
         {
            imageAspect   /*aspectMask*/,
            0             /*baseMipLevel*/,
            m_MIPLevels   /*levelCount*/,
            0             /*baseArrayLevel*/,
            m_Layers      /*layerCount*/
         }             /*subresourceRange*/
      };
      m_ImageView = m_Device->GetVkDevice().createImageView(ci);
      ci.subresourceRange.setLevelCount(1);
      for (uint32_t level = 0; level < m_MIPLevels; ++level) {
         ci.subresourceRange.setBaseMipLevel(level);
         m_MIPImageViews.emplace_back(m_Device->GetVkDevice().createImageView(ci));
      }
   }


   void VulkanImage::DestroyImageViews() {
      if (m_Device) {
         for (auto& imageView : m_MIPImageViews) {
            m_Device->GetVkDevice().destroy(imageView);
         }
         m_MIPImageViews.clear();
         if (m_ImageView) {
            m_Device->GetVkDevice().destroy(m_ImageView);
            m_ImageView = nullptr;
         }
      }
   }


   vk::ImageView VulkanImage::GetVkImageView() const {
      return m_ImageView;
   }


   vk::ImageView VulkanImage::GetVkImageView(const uint32_t mipLevel) const {
      PKZL_CORE_ASSERT(mipLevel < m_MIPImageViews.size(), "Attempted to access image view for invalid mip level!");
      return m_MIPImageViews[mipLevel];
   }


   vk::ImageMemoryBarrier VulkanImage::Barrier(const vk::ImageLayout oldLayout, const vk::ImageLayout newLayout, uint32_t baseMipLevel, uint32_t levelCount, uint32_t baseArrayLayer, uint32_t layerCount) const {
      vk::ImageMemoryBarrier barrier = {
         {}                                  /*srcAccessMask*/,
         {}                                  /*dstAccessMask*/,
         oldLayout                           /*oldLayout*/,
         newLayout                           /*newLayout*/,
         VK_QUEUE_FAMILY_IGNORED             /*srcQueueFamilyIndex*/,
         VK_QUEUE_FAMILY_IGNORED             /*dstQueueFamilyIndex*/,
         m_Image                             /*image*/,
         {
            vk::ImageAspectFlagBits::eColor          /*aspectMask*/,
            baseMipLevel                             /*baseMipLevel*/,
            levelCount == 0? m_MIPLevels: levelCount /*levelCount*/,
            baseArrayLayer                           /*baseArrayLayer*/,
            layerCount == 0? m_Layers : layerCount   /*layerCount*/
         }                                   /*subresourceRange*/
      };

      switch (m_Format) {
         case vk::Format::eD32Sfloat:
         case vk::Format::eD16Unorm:
            barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eDepth;
            break;
         case vk::Format::eD32SfloatS8Uint:
         case vk::Format::eD24UnormS8Uint:
         case vk::Format::eD16UnormS8Uint:
            barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eDepth | vk::ImageAspectFlagBits::eStencil;
            break;
         case vk::Format::eS8Uint:
            barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eStencil;
            break;
      }

      switch (oldLayout) {
         case vk::ImageLayout::eUndefined:
            switch (newLayout) {
               case vk::ImageLayout::eTransferDstOptimal:
                  barrier.srcAccessMask = {};
                  barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;
                  break;
               case vk::ImageLayout::eDepthAttachmentOptimal:
                  barrier.srcAccessMask = {};
                  barrier.dstAccessMask = vk::AccessFlagBits::eDepthStencilAttachmentRead | vk::AccessFlagBits::eDepthStencilAttachmentWrite;
                  break;
               case vk::ImageLayout::eGeneral:
                  barrier.srcAccessMask = {};
                  barrier.dstAccessMask = {};
                  break;
               default:
                  PKZL_CORE_ASSERT(false, "unsupported layout transition!");
            }
            break;

         case vk::ImageLayout::eGeneral:
            switch (newLayout) {
               case vk::ImageLayout::eTransferDstOptimal:
                  barrier.srcAccessMask = {};
                  barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;
                  break;
               case vk::ImageLayout::eShaderReadOnlyOptimal:
                  barrier.srcAccessMask = {};
                  barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;
                  break;
               default:
                  PKZL_CORE_ASSERT(false, "unsupported layout transition!");
            }
            break;

         case vk::ImageLayout::eTransferSrcOptimal:
            switch (newLayout) {
               case vk::ImageLayout::eShaderReadOnlyOptimal:
                  barrier.srcAccessMask = vk::AccessFlagBits::eTransferRead;
                  barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;
                  break;
               default:
                  PKZL_CORE_ASSERT(false, "unsupported layout transition!");
            }
            break;

         case vk::ImageLayout::eTransferDstOptimal:
            switch (newLayout) {
               case vk::ImageLayout::eShaderReadOnlyOptimal:
                  barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
                  barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;
                  break;
               case vk::ImageLayout::eGeneral:
                  barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
                  barrier.dstAccessMask = {};
                  break;
               default:
                  PKZL_CORE_ASSERT(false, "unsupported layout transition!");
            }
            break;

         case vk::ImageLayout::eDepthStencilAttachmentOptimal:
            switch (newLayout) {
               case vk::ImageLayout::eShaderReadOnlyOptimal:
                  barrier.srcAccessMask = vk::AccessFlagBits::eDepthStencilAttachmentRead | vk::AccessFlagBits::eDepthStencilAttachmentWrite;
                  barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;
                  break;
               default:
                  PKZL_CORE_ASSERT(false, "unsupported layout transition!");
            }
            break;

         case vk::ImageLayout::eShaderReadOnlyOptimal:
            switch (newLayout) {
               case vk::ImageLayout::eTransferSrcOptimal:
                  barrier.srcAccessMask = {};
                  barrier.dstAccessMask = vk::AccessFlagBits::eTransferRead;
                  break;
               default:
                  PKZL_CORE_ASSERT(false, "unsupported layout transition!");
            }
            break;

         default:
            PKZL_CORE_ASSERT(false, "unsupported layout transition!");
      }

      return barrier;
   }


   void VulkanImage::CopyFromBuffer(vk::Buffer buffer) {
      m_Device->SubmitSingleTimeCommands(m_Device->GetTransferQueue(), [this, buffer] (vk::CommandBuffer cmd) {
         vk::BufferImageCopy region = {
            0                                    /*bufferOffset*/,
            0                                    /*bufferRowLength*/,
            0                                    /*bufferImageHeight*/,
            vk::ImageSubresourceLayers {
               vk::ImageAspectFlagBits::eColor      /*aspectMask*/,
               0                                    /*mipLevel*/,
               0                                    /*baseArrayLayer*/,
               m_Layers                             /*layerCount*/
            }                                    /*imageSubresource*/,
            {0, 0, 0}                            /*imageOffset*/,
            {m_Width, m_Height, m_Depth}         /*imageExtent*/
         };
         cmd.copyBufferToImage(buffer, m_Image, vk::ImageLayout::eTransferDstOptimal, region);
      });
   }


   void VulkanImage::CopyFromImage(const VulkanImage& image, const vk::ImageCopy& region) {
      m_Device->SubmitSingleTimeCommands(m_Device->GetTransferQueue(), [this, &image, &region] (vk::CommandBuffer cmd) {
         std::array<vk::ImageMemoryBarrier, 2> beforeCopyBarriers = {
            image.Barrier(vk::ImageLayout::eShaderReadOnlyOptimal, vk::ImageLayout::eTransferSrcOptimal, region.srcSubresource.mipLevel, 1, region.srcSubresource.baseArrayLayer, region.srcSubresource.layerCount),
            Barrier(vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal, region.dstSubresource.mipLevel, 1, region.dstSubresource.baseArrayLayer, region.dstSubresource.layerCount)
         };

         std::array<vk::ImageMemoryBarrier, 2> afterCopyBarriers = {
            image.Barrier(vk::ImageLayout::eTransferSrcOptimal, vk::ImageLayout::eShaderReadOnlyOptimal, region.srcSubresource.mipLevel, 1, region.srcSubresource.baseArrayLayer, region.srcSubresource.layerCount),
            Barrier(vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eGeneral, region.dstSubresource.mipLevel, 1, region.dstSubresource.baseArrayLayer, region.dstSubresource.layerCount)
         };

         cmd.pipelineBarrier(vk::PipelineStageFlagBits::eTopOfPipe, vk::PipelineStageFlagBits::eTransfer, {}, nullptr, nullptr, beforeCopyBarriers);
         cmd.copyImage(image.GetVkImage(), vk::ImageLayout::eTransferSrcOptimal, m_Image, vk::ImageLayout::eTransferDstOptimal, region);
         cmd.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eComputeShader | vk::PipelineStageFlagBits::eFragmentShader, {}, nullptr, nullptr, afterCopyBarriers);
      });

   }


   void VulkanImage::GenerateMipmap() {
      // Check if image format supports linear blitting
      vk::FormatProperties formatProperties = m_Device->GetVkPhysicalDevice().getFormatProperties(m_Format);
      if (!(formatProperties.optimalTilingFeatures & vk::FormatFeatureFlagBits::eSampledImageFilterLinear)) {
         throw std::runtime_error {"texture image format does not support linear blitting!"};
      }

      m_Device->SubmitSingleTimeCommands(m_Device->GetComputeQueue(), [this] (vk::CommandBuffer cmd) {
         vk::ImageMemoryBarrier barrier = {
            {}                                   /*srcAccessMask*/,
            {}                                   /*dstAccessMask*/,
            vk::ImageLayout::eUndefined          /*oldLayout*/,
            vk::ImageLayout::eUndefined          /*newLayout*/,
            VK_QUEUE_FAMILY_IGNORED              /*srcQueueFamilyIndex*/,
            VK_QUEUE_FAMILY_IGNORED              /*dstQueueFamilyIndex*/,
            m_Image                              /*image*/,
            vk::ImageSubresourceRange {
               {vk::ImageAspectFlagBits::eColor}    /*aspectMask*/,
               0                                    /*baseMipLevel*/,
               1                                    /*levelCount*/,
               0                                    /*baseArrayLayer*/,
               m_Layers                             /*layerCount*/
            }                                    /*subresourceRange*/
         };

         int32_t mipWidth = m_Width;
         int32_t mipHeight = m_Height;
         int32_t mipDepth = m_Depth;

         for (uint32_t i = 1; i < m_MIPLevels; ++i) {
            barrier.subresourceRange.baseMipLevel = i - 1;
            barrier.oldLayout = vk::ImageLayout::eTransferDstOptimal;
            barrier.newLayout = vk::ImageLayout::eTransferSrcOptimal;
            barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
            barrier.dstAccessMask = vk::AccessFlagBits::eTransferRead;
            cmd.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eTransfer, {}, nullptr, nullptr, barrier);

            vk::ImageBlit blit;
            blit.srcOffsets[0] = vk::Offset3D {0, 0, 0};
            blit.srcOffsets[1] = vk::Offset3D {mipWidth, mipHeight, mipDepth};
            blit.srcSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
            blit.srcSubresource.mipLevel = i - 1;
            blit.srcSubresource.baseArrayLayer = 0;
            blit.srcSubresource.layerCount = m_Layers;;
            blit.dstOffsets[0] = vk::Offset3D {0, 0, 0};
            blit.dstOffsets[1] = vk::Offset3D {mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, mipDepth > 1 ? mipDepth / 2 : 1};
            blit.dstSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
            blit.dstSubresource.mipLevel = i;
            blit.dstSubresource.baseArrayLayer = 0;
            blit.dstSubresource.layerCount = m_Layers;;
            cmd.blitImage(m_Image, vk::ImageLayout::eTransferSrcOptimal, m_Image, vk::ImageLayout::eTransferDstOptimal, blit, vk::Filter::eLinear);

            barrier.oldLayout = vk::ImageLayout::eTransferSrcOptimal;
            barrier.newLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
            barrier.srcAccessMask = vk::AccessFlagBits::eTransferRead;
            barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;
            cmd.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eFragmentShader, {}, nullptr, nullptr, barrier);

            if (mipWidth > 1) {
               mipWidth /= 2;
            }
            if (mipHeight > 1) {
               mipHeight /= 2;
            }
            if (mipDepth > 1) {
               mipDepth /= 2;
            }
         }
         barrier.subresourceRange.baseMipLevel = m_MIPLevels - 1;
         barrier.oldLayout = vk::ImageLayout::eTransferDstOptimal;
         barrier.newLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
         barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
         barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;
         cmd.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eFragmentShader, {}, nullptr, nullptr, barrier);
      });
   }

}
