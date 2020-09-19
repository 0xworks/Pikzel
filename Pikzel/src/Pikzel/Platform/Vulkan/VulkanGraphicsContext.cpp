#include "VulkanGraphicsContext.h"
#include "VulkanUtility.h"

namespace Pikzel {

   VulkanGraphicsContext::VulkanGraphicsContext(std::shared_ptr<VulkanDevice> device) : m_Device {device} {}


   VulkanGraphicsContext::~VulkanGraphicsContext() {}


   vk::RenderPass VulkanGraphicsContext::GetVkRenderPass() const {
      return m_RenderPass;
   }


   vk::PipelineCache VulkanGraphicsContext::GetVkPipelineCache() const {
      return m_PipelineCache;
   }


   void VulkanGraphicsContext::CreateDepthStencil() {
      // TODO anti-aliasing
      m_DepthFormat = FindSupportedFormat(
         m_Device->GetVkPhysicalDevice(),
         {vk::Format::eD32SfloatS8Uint, vk::Format::eD32Sfloat, vk::Format::eD24UnormS8Uint, vk::Format::eD16UnormS8Uint, vk::Format::eD16Unorm},
         vk::ImageTiling::eOptimal,
         vk::FormatFeatureFlagBits::eDepthStencilAttachment
      );
      m_DepthImage = std::make_unique<VulkanImage>(m_Device, m_Extent.width, m_Extent.height, 1, vk::SampleCountFlagBits::e1, m_DepthFormat, vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eDepthStencilAttachment, vk::MemoryPropertyFlagBits::eDeviceLocal);
      m_DepthImage->CreateImageView(m_DepthFormat, vk::ImageAspectFlagBits::eDepth);
   }


   void VulkanGraphicsContext::DestroyDepthStencil() {
      m_DepthImage = nullptr;
   }


   vk::RenderPass VulkanGraphicsContext::CreateRenderPass() {
      std::vector<vk::AttachmentDescription> attachments = {
         {
            {}                                         /*flags*/,
            m_Format                                   /*format*/,
            vk::SampleCountFlagBits::e1                /*samples*/,   // TODO: anti-aliasing
            vk::AttachmentLoadOp::eClear               /*loadOp*/,
            vk::AttachmentStoreOp::eStore              /*storeOp*/,
            vk::AttachmentLoadOp::eDontCare            /*stencilLoadOp*/,
            vk::AttachmentStoreOp::eDontCare           /*stencilStoreOp*/,
            vk::ImageLayout::eUndefined                /*initialLayout*/,
            vk::ImageLayout::ePresentSrcKHR            /*finalLayout*/     // anti-aliasing = vk::ImageLayout::eColorAttachmentOptimal here
         },
         {
            {}                                              /*flags*/,
            m_DepthFormat                                   /*format*/,
            vk::SampleCountFlagBits::e1                     /*samples*/,   // TODO: anti-aliasing
            vk::AttachmentLoadOp::eClear                    /*loadOp*/,
            vk::AttachmentStoreOp::eStore                   /*storeOp*/,
            vk::AttachmentLoadOp::eClear                    /*stencilLoadOp*/,
            vk::AttachmentStoreOp::eDontCare                /*stencilStoreOp*/,
            vk::ImageLayout::eUndefined                     /*initialLayout*/,
            vk::ImageLayout::eDepthStencilAttachmentOptimal /*finalLayout*/
         }
         //{
         //    {}                                              /*flags*/,
         //    format                                          /*format*/,
         //    vk::SampleCountFlagBits::e1                     /*samples*/,
         //    vk::AttachmentLoadOp::eDontCare                 /*loadOp*/,
         //    vk::AttachmentStoreOp::eStore                   /*storeOp*/,
         //    vk::AttachmentLoadOp::eDontCare                 /*stencilLoadOp*/,
         //    vk::AttachmentStoreOp::eDontCare                /*stencilStoreOp*/,
         //    vk::ImageLayout::eUndefined                     /*initialLayout*/,
         //    vk::ImageLayout::ePresentSrcKHR                 /*finalLayout*/
         //}
      };

      vk::AttachmentReference colorAttachmentRef = {
         0,
         vk::ImageLayout::eColorAttachmentOptimal
      };

      vk::AttachmentReference depthAttachmentRef = {
         1,
         vk::ImageLayout::eDepthStencilAttachmentOptimal
      };

      //    vk::AttachmentReference resolveAttachmentRef = {
      //       2,
      //       vk::ImageLayout::eColorAttachmentOptimal
      //    };

      vk::SubpassDescription subpass = {
         {}                               /*flags*/,
         vk::PipelineBindPoint::eGraphics /*pipelineBindPoint*/,
         0                                /*inputAttachmentCount*/,
         nullptr                          /*pInputAttachments*/,
         1                                /*colorAttachmentCount*/,
         &colorAttachmentRef              /*pColorAttachments*/,
         nullptr, //&resolveAttachmentRef /*pResolveAttachments*/,
         &depthAttachmentRef              /*pDepthStencilAttachment*/,
         0                                /*preserveAttachmentCount*/,
         nullptr                          /*pPreserveAttachments*/
      };

      std::vector<vk::SubpassDependency> dependencies = {
         {
            VK_SUBPASS_EXTERNAL                                                                    /*srcSubpass*/,
            0                                                                                      /*dstSubpass*/,
            vk::PipelineStageFlagBits::eBottomOfPipe                                               /*srcStageMask*/,
            vk::PipelineStageFlagBits::eColorAttachmentOutput                                      /*dstStageMask*/,
            vk::AccessFlagBits::eMemoryRead                                                        /*srcAccessMask*/,
            vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite   /*dstAccessMask*/,
            vk::DependencyFlagBits::eByRegion                                                      /*dependencyFlags*/
         },
         {
            0                                                                                      /*srcSubpass*/,
            VK_SUBPASS_EXTERNAL                                                                    /*dstSubpass*/,
            vk::PipelineStageFlagBits::eColorAttachmentOutput                                      /*srcStageMask*/,
            vk::PipelineStageFlagBits::eBottomOfPipe                                               /*dstStageMask*/,
            vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite   /*srcAccessMask*/,
            vk::AccessFlagBits::eMemoryRead                                                        /*dstAccessMask*/,
            vk::DependencyFlagBits::eByRegion                                                      /*dependencyFlags*/
         }
      };

      return m_Device->GetVkDevice().createRenderPass({
         {}                                         /*flags*/,
         static_cast<uint32_t>(attachments.size())  /*attachmentCount*/,
         attachments.data()                         /*pAttachments*/,
         1                                          /*subpassCount*/,
         &subpass                                   /*pSubpasses*/,
         static_cast<uint32_t>(dependencies.size()) /*dependencyCount*/,
         dependencies.data()                        /*pDependencies*/
      });
   }


   void VulkanGraphicsContext::DestroyRenderPass(vk::RenderPass renderPass) {
      if (m_Device) {
         if (renderPass) {
            m_Device->GetVkDevice().destroy(renderPass);
            renderPass = nullptr;
         }
      }
   }


   vk::DescriptorPool VulkanGraphicsContext::CreateDescriptorPool(const vk::ArrayProxy<DescriptorBinding>& descriptorBindings, size_t maxSets) {
      std::vector<vk::DescriptorPoolSize> poolSizes;
      poolSizes.reserve(descriptorBindings.size());

      for (const auto& binding : descriptorBindings) {
         poolSizes.emplace_back(binding.Type, static_cast<uint32_t>(binding.DescriptorCount * maxSets));
      }

      vk::DescriptorPoolCreateInfo descriptorPoolCI = {
         vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet   /*flags*/,
         static_cast<uint32_t>(maxSets)                         /*maxSets*/,
         static_cast<uint32_t>(poolSizes.size())                /*poolSizeCount*/,
         poolSizes.data()                                       /*pPoolSizes*/
      };
      return m_Device->GetVkDevice().createDescriptorPool(descriptorPoolCI);
   }


   void VulkanGraphicsContext::DestroyDescriptorPool(vk::DescriptorPool descriptorPool) {
      if (m_Device) {
         if (descriptorPool) {
            m_Device->GetVkDevice().destroy(descriptorPool);
            descriptorPool = nullptr;
         }
      }
   }


   void VulkanGraphicsContext::CreateCommandPool() {
      m_CommandPool = m_Device->GetVkDevice().createCommandPool({
         vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
         m_Device->GetGraphicsQueueFamilyIndex()
      });
   }


   void VulkanGraphicsContext::DestroyCommandPool() {
      if (m_Device && m_CommandPool) {
         m_Device->GetVkDevice().destroy(m_CommandPool);
         m_CommandPool = nullptr;
      }
   }


   void VulkanGraphicsContext::CreateCommandBuffers(const uint32_t commandBufferCount) {
      m_CommandBuffers = m_Device->GetVkDevice().allocateCommandBuffers({
         m_CommandPool                      /*commandPool*/,
         vk::CommandBufferLevel::ePrimary   /*level*/,
         commandBufferCount                 /*commandBufferCount*/
      });
   }


   void VulkanGraphicsContext::DestroyCommandBuffers() {
      if (m_Device && m_CommandPool) {
         m_Device->GetVkDevice().freeCommandBuffers(m_CommandPool, m_CommandBuffers);
         m_CommandBuffers.clear();
      }
   }


   void VulkanGraphicsContext::CreatePipelineCache() {
      m_PipelineCache = m_Device->GetVkDevice().createPipelineCache({});
   }


   void VulkanGraphicsContext::DestroyPipelineCache() {
      if (m_Device && m_PipelineCache) {
         m_Device->GetVkDevice().destroy(m_PipelineCache);
      }
   }

}
