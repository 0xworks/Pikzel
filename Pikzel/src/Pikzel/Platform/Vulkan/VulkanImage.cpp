#include "VulkanImage.h"
#include "VulkanUtility.h"

namespace Pikzel {

   VulkanImage::VulkanImage(std::shared_ptr<VulkanDevice> device, const uint32_t width, const uint32_t height, const uint32_t mipLevels, vk::SampleCountFlagBits numSamples, const vk::Format format, const vk::ImageTiling tiling, const vk::ImageUsageFlags usage, const vk::MemoryPropertyFlags properties, const vk::ImageCreateFlags createFlags)
   : m_Device {device}
   , m_Format {format}
   , m_Extent {width, height}
   , m_MIPLevels {mipLevels}
   , m_IsCubeCompatible{createFlags & vk::ImageCreateFlagBits::eCubeCompatible}
   {

      m_Image = m_Device->GetVkDevice().createImage({
         createFlags                 /*flags*/,
         vk::ImageType::e2D          /*imageType*/,
         format                      /*format*/,
         {width, height, 1}          /*extent*/,
         mipLevels                   /*mipLevels*/,
         m_IsCubeCompatible? 6u : 1u /*arrayLayers*/,
         numSamples                  /*samples*/,
         tiling                      /*tiling*/,
         usage                       /*usage*/,
         vk::SharingMode::eExclusive /*sharingMode*/,
         0                           /*queueFamilyIndexCount*/,
         nullptr                     /*pQueueFamilyIndices*/,
         vk::ImageLayout::eUndefined /*initialLayout*/
      });

      vk::MemoryRequirements memRequirements = m_Device->GetVkDevice().getImageMemoryRequirements(m_Image);
      m_Memory = m_Device->GetVkDevice().allocateMemory({
         memRequirements.size                                                                         /*allocationSize*/,
         FindMemoryType(m_Device->GetVkPhysicalDevice(), memRequirements.memoryTypeBits, properties)  /*memoryTypeIndex*/
      });
      m_Device->GetVkDevice().bindImageMemory(m_Image, m_Memory, 0);
   }


   VulkanImage::VulkanImage(std::shared_ptr<VulkanDevice> device, const vk::Image& image, vk::Format format, vk::Extent2D extent)
   : m_Device {device}
   , m_Image {image}
   , m_Format {format}
   , m_Extent {extent}
   , m_MIPLevels {1}
   , m_IsCubeCompatible {false}
   {}


   VulkanImage::~VulkanImage() {
      if (m_Device) {
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


   vk::Image VulkanImage::GetVkImage() const {
      return m_Image;
   }


   vk::Format VulkanImage::GetVkFormat() const {
      return m_Format;
   }


   vk::Extent2D VulkanImage::GetVkExtent() const {
      return m_Extent;
   }


   uint32_t VulkanImage::GetMIPLevels() const {
      return m_MIPLevels;
   }


   void VulkanImage::CreateImageView(const vk::Format format, const vk::ImageAspectFlags imageAspect) {
      m_ImageView = m_Device->GetVkDevice().createImageView({
         {}                                                                    /*flags*/,
         m_Image                                                               /*image*/,
         m_IsCubeCompatible? vk::ImageViewType::eCube : vk::ImageViewType::e2D /*viewType*/,
         format                                                                /*format*/,
         {}                                                                    /*components*/,
         {
            imageAspect                                                           /*aspectMask*/,
            0                                                                     /*baseMipLevel*/,
            m_MIPLevels                                                           /*levelCount*/,
            0                                                                     /*baseArrayLevel*/,
            m_IsCubeCompatible ? 6u : 1u                                          /*layerCount*/
         }                                                                     /*subresourceRange*/
      });
   }


   void VulkanImage::DestroyImageView() {
      if (m_Device && m_ImageView) {
         m_Device->GetVkDevice().destroy(m_ImageView);
         m_ImageView = nullptr;
      }
   }


   vk::ImageView VulkanImage::GetVkImageView() const {
      return m_ImageView;
   }


   void VulkanImage::TransitionImageLayout(const vk::ImageLayout oldLayout, const vk::ImageLayout newLayout) {
      m_Device->SubmitSingleTimeCommands(m_Device->GetComputeQueue(), [this, oldLayout, newLayout] (vk::CommandBuffer cmd) {
         vk::ImageMemoryBarrier barrier = {
            {}                                  /*srcAccessMask*/,
            {}                                  /*dstAccessMask*/,
            oldLayout                           /*oldLayout*/,
            newLayout                           /*newLayout*/,
            VK_QUEUE_FAMILY_IGNORED             /*srcQueueFamilyIndex*/,
            VK_QUEUE_FAMILY_IGNORED             /*dstQueueFamilyIndex*/,
            m_Image                             /*image*/,
            {
               vk::ImageAspectFlagBits::eColor     /*aspectMask*/,
               0                                   /*baseMipLevel*/,
               m_MIPLevels                         /*levelCount*/,
               0                                   /*baseArrayLayer*/,
               m_IsCubeCompatible? 6u : 1u         /*layerCount*/
            }                                   /*subresourceRange*/
         };

         vk::PipelineStageFlags sourceStage;
         vk::PipelineStageFlags destinationStage;

         if (oldLayout == (vk::ImageLayout::eUndefined) && (newLayout == vk::ImageLayout::eTransferDstOptimal)) {
            barrier.srcAccessMask = {};
            barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;
            sourceStage = vk::PipelineStageFlagBits::eTopOfPipe;
            destinationStage = vk::PipelineStageFlagBits::eTransfer;
         } else if ((oldLayout == vk::ImageLayout::eUndefined) && (newLayout == vk::ImageLayout::eGeneral)) {
            barrier.srcAccessMask = {};
            barrier.dstAccessMask = {};
            sourceStage = vk::PipelineStageFlagBits::eAllCommands;
            destinationStage = vk::PipelineStageFlagBits::eAllCommands;
         } else if ((oldLayout == vk::ImageLayout::eGeneral) && (newLayout == vk::ImageLayout::eTransferDstOptimal)) {
            barrier.srcAccessMask = {};
            barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;
            sourceStage = vk::PipelineStageFlagBits::eAllCommands;
            destinationStage = vk::PipelineStageFlagBits::eTransfer;
         }  else if ((oldLayout == vk::ImageLayout::eTransferDstOptimal) && (newLayout == vk::ImageLayout::eShaderReadOnlyOptimal)) {
            barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
            barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;
            sourceStage = vk::PipelineStageFlagBits::eTransfer;
            destinationStage = vk::PipelineStageFlagBits::eFragmentShader;
         } else {
            throw std::invalid_argument("unsupported layout transition!");
         }

         cmd.pipelineBarrier(sourceStage, destinationStage, {}, nullptr, nullptr, barrier);
      });
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
               m_IsCubeCompatible? 6u: 1u           /*layerCount*/
            }                                    /*imageSubresource*/,
            {0, 0, 0}                            /*imageOffset*/,
            {m_Extent.width, m_Extent.height, 1}     /*imageExtent*/
         };
         cmd.copyBufferToImage(buffer, m_Image, vk::ImageLayout::eTransferDstOptimal, region);
      });
   }


   void VulkanImage::GenerateMIPMaps() {
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
               m_IsCubeCompatible? 6u : 1u          /*layerCount*/
            }                                    /*subresourceRange*/
         };

         int32_t mipWidth = static_cast<int32_t>(m_Extent.width);
         int32_t mipHeight = static_cast<int32_t>(m_Extent.height);

         for (uint32_t i = 1; i < m_MIPLevels; ++i) {
            barrier.subresourceRange.baseMipLevel = i - 1;
            barrier.oldLayout = vk::ImageLayout::eTransferDstOptimal;
            barrier.newLayout = vk::ImageLayout::eTransferSrcOptimal;
            barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
            barrier.dstAccessMask = vk::AccessFlagBits::eTransferRead;
            cmd.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eTransfer, {}, nullptr, nullptr, barrier);

            vk::ImageBlit blit;
            blit.srcOffsets[0] = vk::Offset3D {0, 0, 0};
            blit.srcOffsets[1] = vk::Offset3D {mipWidth, mipHeight, 1};
            blit.srcSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
            blit.srcSubresource.mipLevel = i - 1;
            blit.srcSubresource.baseArrayLayer = 0;
            blit.srcSubresource.layerCount = m_IsCubeCompatible? 6u: 1u;
            blit.dstOffsets[0] = vk::Offset3D {0, 0, 0};
            blit.dstOffsets[1] = vk::Offset3D {mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1};
            blit.dstSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
            blit.dstSubresource.mipLevel = i;
            blit.dstSubresource.baseArrayLayer = 0;
            blit.dstSubresource.layerCount = m_IsCubeCompatible? 6u: 1u;
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
