#include "vkpch.h"
#include "VulkanImageGC.h"

#include "VulkanUtility.h"

namespace Pikzel {

   VulkanImageGC::VulkanImageGC(std::shared_ptr<VulkanDevice> device, VulkanImage* image)
   : VulkanGraphicsContext {device}
   , m_Image {image} 
   {
      m_Format = m_Image->GetFormat();
      m_Extent = m_Image->GetExtent(); // TODO: what if image is resized?
      if (!m_Image->GetImageView()) {
         m_Image->CreateImageView(m_Image->GetFormat(), vk::ImageAspectFlagBits::eColor, m_Image->GetMIPLevels());
      }
      if (!m_Image->GetSampler()) {
         m_Image->CreateSampler();
      }
      CreateDepthStencil();
      m_RenderPass = CreateRenderPass();
      CreateFrameBuffer();
      CreateCommandPool();
      CreateCommandBuffers(1);
      CreateSyncObjects();
      // CreatePipelineCache(); // needed ?
   }


   VulkanImageGC::~VulkanImageGC() {
      DestroyPipelineCache();
      DestroySyncObjects();
      DestroyCommandBuffers();
      DestroyCommandPool();
      DestroyFrameBuffer();
      DestroyRenderPass(m_RenderPass);
      DestroyDepthStencil();
   }


   void VulkanImageGC::BeginFrame() {
      m_Device->GetVkDevice().waitForFences(m_ImageFence, true, UINT64_MAX);
      vk::CommandBufferBeginInfo commandBufferBI = {
         vk::CommandBufferUsageFlagBits::eSimultaneousUse
      };
      m_CommandBuffers[0].begin(commandBufferBI);

      // TODO: Is this the best place/way to begin render pass?
      //       What if you want more than one render pass?
      std::array<vk::ClearValue, 2> clearValues = {
         vk::ClearColorValue {std::array<float,4>{1.0f, 0.0f, 0.0f, 1.0f}},
         vk::ClearDepthStencilValue {1.0f, 0}
      };

      vk::RenderPassBeginInfo renderPassBI = {
         m_RenderPass           /*renderPass*/,
         m_FrameBuffer          /*framebuffer*/,
         { {0,0}, m_Extent }    /*renderArea*/,
         2                      /*clearValueCount*/,
         clearValues.data()     /*pClearValues*/
      };

      vk::CommandBuffer commandBuffer = m_CommandBuffers[0];
      commandBuffer.beginRenderPass(renderPassBI, vk::SubpassContents::eInline);

   }


   void VulkanImageGC::EndFrame() {
      m_CommandBuffers[0].endRenderPass();
      m_CommandBuffers[0].end();
      vk::PipelineStageFlags waitStages[] = {{vk::PipelineStageFlagBits::eColorAttachmentOutput}};
      vk::SubmitInfo si = {
         0                      /*waitSemaphoreCount*/,
         nullptr                /*pWaitSemaphores*/,
         nullptr                /*pWaitDstStageMask*/,
         1                      /*commandBufferCount*/,
         &m_CommandBuffers[0]   /*pCommandBuffers*/,
         0                      /*signalSemaphoreCount*/,
         nullptr                /*pSignalSemaphores*/
      };
      m_Device->GetVkDevice().resetFences(m_ImageFence);
      m_Device->GetGraphicsQueue().submit(si, m_ImageFence);
   }


   void VulkanImageGC::SwapBuffers() {
      // There's no buffers to "swap". We just wait for the render that we submitted in EndFrame() to complete
      m_Device->GetVkDevice().waitForFences(m_ImageFence, true, UINT64_MAX);
   }


   void VulkanImageGC::CreateFrameBuffer() {
      PKZL_CORE_ASSERT(m_Image->GetImageView(), "VulkanImageGC::CreateFrameBuffer() depends on image view, which is null!");
      PKZL_CORE_ASSERT(m_DepthImage->GetImageView(), "VulkanImageGC::CreateFrameBuffer() depends on depth image view, which is null!");
      std::array<vk::ImageView, 2> attachments = {
         m_Image->GetImageView(),
         m_DepthImage->GetImageView()
      };
      vk::FramebufferCreateInfo ci = {
         {}                                        /*flags*/,
         m_RenderPass                              /*renderPass*/,
         static_cast<uint32_t>(attachments.size()) /*attachmentCount*/,
         attachments.data()                        /*pAttachments*/,
         m_Extent.width                            /*width*/,
         m_Extent.height                           /*height*/,
         1                                         /*layers*/
      };
      m_FrameBuffer = m_Device->GetVkDevice().createFramebuffer(ci);
   }


   void VulkanImageGC::DestroyFrameBuffer() {
      if (m_Device && m_FrameBuffer) {
         m_Device->GetVkDevice().destroy(m_FrameBuffer);
         m_FrameBuffer = nullptr;
      }
   }


   void VulkanImageGC::CreateSyncObjects() {
      vk::FenceCreateInfo ci = {
         vk::FenceCreateFlagBits::eSignaled   /*flags*/
      };
      m_ImageFence = m_Device->GetVkDevice().createFence(ci);
   }


   void VulkanImageGC::DestroySyncObjects() {
      if (m_Device && m_ImageFence) {
         m_Device->GetVkDevice().destroy(m_ImageFence);
         m_ImageFence = nullptr;
      }
   }

}
