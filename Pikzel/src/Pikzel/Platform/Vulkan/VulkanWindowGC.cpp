#include "vkpch.h"
#include "VulkanWindowGC.h"

#include "SwapChainSupportDetails.h"
#include "VulkanUtility.h"

#include "Pikzel/Events/EventDispatcher.h"

namespace Pikzel {

   VulkanWindowGC::VulkanWindowGC(std::shared_ptr<VulkanDevice> device, GLFWwindow* window)
   : VulkanGraphicsContext {device}
   , m_Window(window)
   {
      CreateSurface();
      CreateSwapChain();
      CreateImageViews();
      CreateDepthStencil();
      m_RenderPass = CreateRenderPass();
      CreateFrameBuffers();

      CreateCommandPool();
      CreateCommandBuffers(static_cast<uint32_t>(m_SwapChainImages.size()));
      CreateSyncObjects();
      CreatePipelineCache();

      EventDispatcher::Connect<WindowResizeEvent, &VulkanWindowGC::OnWindowResize>(*this);
   }


   VulkanWindowGC::~VulkanWindowGC() {
      DestroyPipelineCache();
      DestroySyncObjects();
      DestroyCommandBuffers();
      DestroyCommandPool();
      DestroyFrameBuffers();
      DestroyRenderPass(m_RenderPass);
      DestroyDepthStencil();
      DestroyImageViews();
      DestroySwapChain(m_SwapChain);
      DestroySurface();
   }


   void VulkanWindowGC::BeginFrame() {
      m_Device->GetVkDevice().waitForFences(m_InFlightFences[m_CurrentFrame], true, UINT64_MAX);
      auto rv = m_Device->GetVkDevice().acquireNextImageKHR(m_SwapChain, UINT64_MAX, m_ImageAvailableSemaphores[m_CurrentFrame], nullptr);

      if (rv.result == vk::Result::eErrorOutOfDateKHR) {
         RecreateSwapChain();
         return;
      } else if ((rv.result != vk::Result::eSuccess) && (rv.result != vk::Result::eSuboptimalKHR)) {
         throw std::runtime_error("failed to acquire swap chain image!");
      }

      // acquireNextImage returns as soon as it has decided which image is the next one.
      // That doesn't necessarily mean the image is available for either the CPU or the GPU to start doing stuff to it,
      // it's just that we now know which image is *going to be* the next one.
      // The semaphore that was passed in gets signaled when the image really is available (so need to tell the GPU to wait on that sempahore
      // before doing anything to the image).
      //
      // The CPU also needs to wait.. but on what?
      // That's where our in flight fences come in.  If we know which frame it was that last used this image, then we wait on that frame's fence before
      // queuing up more stuff for the image.
      m_CurrentImage = rv.value;

      if (m_ImagesInFlight[m_CurrentImage]) {
         m_Device->GetVkDevice().waitForFences(m_ImagesInFlight[m_CurrentImage], true, UINT64_MAX);
      }
      // Mark the image as now being in use by this frame
      m_ImagesInFlight[m_CurrentImage] = m_InFlightFences[m_CurrentFrame];

      vk::CommandBufferBeginInfo commandBufferBI = {
         vk::CommandBufferUsageFlagBits::eSimultaneousUse
      };
      m_CommandBuffers[m_CurrentImage].begin(commandBufferBI);

      // TODO: somewhere you need to begin and end a render pass...

   }


   void VulkanWindowGC::EndFrame() {
      m_CommandBuffers[m_CurrentImage].end();
      vk::PipelineStageFlags waitStages[] = {{vk::PipelineStageFlagBits::eColorAttachmentOutput}};
      vk::SubmitInfo si = {
         1                                             /*waitSemaphoreCount*/,
         &m_ImageAvailableSemaphores[m_CurrentFrame]   /*pWaitSemaphores*/,
         waitStages                                    /*pWaitDstStageMask*/,
         1                                             /*commandBufferCount*/,
         &m_CommandBuffers[m_CurrentImage]             /*pCommandBuffers*/,
         1                                             /*signalSemaphoreCount*/,
         &m_RenderFinishedSemaphores[m_CurrentFrame]   /*pSignalSemaphores*/
      };

      m_Device->GetVkDevice().resetFences(m_InFlightFences[m_CurrentFrame]);
      m_Device->GetGraphicsQueue().submit(si, m_InFlightFences[m_CurrentFrame]);
   }


   void VulkanWindowGC::SwapBuffers() {
      vk::PresentInfoKHR pi = {
         1                                            /*waitSemaphoreCount*/,
         &m_RenderFinishedSemaphores[m_CurrentFrame]  /*pWaitSemaphores*/,
         1                                            /*swapchainCount*/,
         &m_SwapChain                                 /*pSwapchains*/,
         &m_CurrentImage                              /*pImageIndices*/,
         nullptr                                      /*pResults*/
      };

      // do not use cpp wrappers here.
      // The problem is that VK_ERROR_OUT_OF_DATE_KHR is an exception in the cpp wrapper, rather
      // than a valid return code.
      // Of course, you could try..catch but that seems quite an inefficient way to do it.
      //auto result = m_PresentQueue.presentKHR(pi);
      auto result = vkQueuePresentKHR(m_Device->GetPresentQueue(), &(VkPresentInfoKHR)pi);
      if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || m_WantResize) {
         RecreateSwapChain();
      } else if (result != VK_SUCCESS) {
         throw std::runtime_error("Failed to present swap chain image!");
      }

      m_CurrentFrame = ++m_CurrentFrame % m_MaxFramesInFlight;
   }


   void VulkanWindowGC::CreateSurface() {
      VkSurfaceKHR surface;
      if (glfwCreateWindowSurface(m_Device->GetVkInstance(), m_Window, nullptr, &surface) != VK_SUCCESS) {
         throw std::runtime_error("failed to create window surface!");
      }
      m_Surface = surface;
   }


   void VulkanWindowGC::DestroySurface() {
      if (m_Device && m_Device->GetVkInstance() && m_Surface) {
         m_Device->GetVkInstance().destroy(m_Surface);
         m_Surface = nullptr;
      }
   }


   vk::SurfaceFormatKHR VulkanWindowGC::SelectSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats) {
      for (const auto& availableFormat : availableFormats) {
         if (availableFormat.format == vk::Format::eB8G8R8A8Unorm && availableFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
            return availableFormat;
         }
      }
      return availableFormats[0];
   }


   vk::PresentModeKHR VulkanWindowGC::SelectPresentMode(const std::vector<vk::PresentModeKHR>& availablePresentModes) {
      for (const auto& availablePresentMode : availablePresentModes) {
         if (availablePresentMode == vk::PresentModeKHR::eMailbox) {
            return availablePresentMode;
         }
      }
      return vk::PresentModeKHR::eFifo;
   }


   void VulkanWindowGC::CreateSwapChain() {
      vk::SwapchainKHR oldSwapChain = m_SwapChain;

      SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(m_Device->GetVkPhysicalDevice(), m_Surface);

      vk::SurfaceFormatKHR surfaceFormat = SelectSurfaceFormat(swapChainSupport.Formats);
      vk::PresentModeKHR presentMode = SelectPresentMode(swapChainSupport.PresentModes);
      m_Format = surfaceFormat.format;
      m_Extent = SelectSwapExtent(swapChainSupport.Capabilities);

      uint32_t imageCount = swapChainSupport.Capabilities.minImageCount;// +1;
      if ((swapChainSupport.Capabilities.maxImageCount > 0) && (imageCount > swapChainSupport.Capabilities.maxImageCount)) {
         imageCount = swapChainSupport.Capabilities.maxImageCount;
      }

      // Find the transformation of the surface
      vk::SurfaceTransformFlagBitsKHR preTransform;
      if (swapChainSupport.Capabilities.supportedTransforms & vk::SurfaceTransformFlagBitsKHR::eIdentity) {
         // We prefer a non-rotated transform
         preTransform = vk::SurfaceTransformFlagBitsKHR::eIdentity;
      } else {
         preTransform = swapChainSupport.Capabilities.currentTransform;
      }

      // Find a supported composite alpha format (not all devices support alpha opaque)
      vk::CompositeAlphaFlagBitsKHR compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
      // Select the first composite alpha format available
      std::array<vk::CompositeAlphaFlagBitsKHR, 4> compositeAlphaFlags = {
         vk::CompositeAlphaFlagBitsKHR::eOpaque,
         vk::CompositeAlphaFlagBitsKHR::ePreMultiplied,
         vk::CompositeAlphaFlagBitsKHR::ePostMultiplied,
         vk::CompositeAlphaFlagBitsKHR::eInherit
      };
      for (const auto& compositeAlphaFlag : compositeAlphaFlags) {
         if (swapChainSupport.Capabilities.supportedCompositeAlpha & compositeAlphaFlag) {
            compositeAlpha = compositeAlphaFlag;
            break;
         };
      }

      vk::SwapchainCreateInfoKHR ci;
      ci.surface = m_Surface;
      ci.minImageCount = imageCount;
      ci.imageFormat = m_Format;
      ci.imageColorSpace = surfaceFormat.colorSpace;
      ci.imageExtent = m_Extent;
      ci.imageArrayLayers = 1;
      ci.imageUsage = vk::ImageUsageFlagBits::eColorAttachment;
      ci.preTransform = preTransform;
      ci.compositeAlpha = compositeAlpha;
      ci.presentMode = presentMode;
      ci.clipped = true;
      ci.oldSwapchain = oldSwapChain;

      uint32_t queueFamilyIndices[] = {m_Device->GetGraphicsQueueFamilyIndex(), m_Device->GetPresentQueueFamilyIndex()};
      if (m_Device->GetGraphicsQueueFamilyIndex() != m_Device->GetPresentQueueFamilyIndex()) {
         ci.imageSharingMode = vk::SharingMode::eConcurrent;
         ci.queueFamilyIndexCount = 2;
         ci.pQueueFamilyIndices = queueFamilyIndices;
      }

      // Enable transfer source on swap chain images if supported
      if (swapChainSupport.Capabilities.supportedUsageFlags & vk::ImageUsageFlagBits::eTransferSrc) {
         ci.imageUsage |= vk::ImageUsageFlagBits::eTransferSrc;
      }

      // Enable transfer destination on swap chain images if supported
      if (swapChainSupport.Capabilities.supportedUsageFlags & vk::ImageUsageFlagBits::eTransferDst) {
         ci.imageUsage |= vk::ImageUsageFlagBits::eTransferDst;
      }

      m_SwapChain = m_Device->GetVkDevice().createSwapchainKHR(ci);
      DestroySwapChain(oldSwapChain);
      std::vector<vk::Image> swapChainImages = m_Device->GetVkDevice().getSwapchainImagesKHR(m_SwapChain);
      for (const auto& image : swapChainImages) {
         m_SwapChainImages.emplace_back(m_Device, image, m_Format, m_Extent);
      }
   }


   void VulkanWindowGC::DestroySwapChain(vk::SwapchainKHR& swapChain) {
      if (m_Device && swapChain) {
         m_SwapChainImages.clear();
         m_Device->GetVkDevice().destroy(swapChain);
         swapChain = nullptr;
      }
   }


   void VulkanWindowGC::CreateImageViews() {
      for (auto& image : m_SwapChainImages) {
         image.CreateImageView(m_Format, vk::ImageAspectFlagBits::eColor, 1);
      }
   }


   void VulkanWindowGC::DestroyImageViews() {
      if (m_Device) {
         for (auto& image : m_SwapChainImages) {
            image.DestroyImageView();
         }
      }
   }


   vk::Extent2D VulkanWindowGC::SelectSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities) {
      if (capabilities.currentExtent.width != UINT32_MAX) {
         return capabilities.currentExtent;
      } else {
         int width;
         int height;
         glfwGetFramebufferSize(m_Window, &width, &height);
         vk::Extent2D actualExtent = {
             static_cast<uint32_t>(width),
             static_cast<uint32_t>(height)
         };

         actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
         actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

         return actualExtent;
      }
   }


   void VulkanWindowGC::CreateFrameBuffers() {
      std::array<vk::ImageView, 2> attachments = {
         nullptr,
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

      m_SwapChainFrameBuffers.reserve(m_SwapChainImages.size());
      for (const auto& swapChainImage : m_SwapChainImages) {
         attachments[0] = swapChainImage.GetImageView();
         m_SwapChainFrameBuffers.push_back(m_Device->GetVkDevice().createFramebuffer(ci));
      }
   }


   void VulkanWindowGC::DestroyFrameBuffers() {
      if (m_Device) {
         for (auto frameBuffer : m_SwapChainFrameBuffers) {
            m_Device->GetVkDevice().destroy(frameBuffer);
         }
         m_SwapChainFrameBuffers.clear();
      }
   }


   void VulkanWindowGC::CreateSyncObjects() {
      m_ImageAvailableSemaphores.reserve(m_MaxFramesInFlight);
      m_RenderFinishedSemaphores.reserve(m_MaxFramesInFlight);
      m_InFlightFences.reserve(m_MaxFramesInFlight);
      m_ImagesInFlight.resize(m_SwapChainImages.size(), nullptr);

      vk::FenceCreateInfo ci = {
         {vk::FenceCreateFlagBits::eSignaled}
      };

      for (uint32_t i = 0; i < m_MaxFramesInFlight; ++i) {
         m_ImageAvailableSemaphores.emplace_back(m_Device->GetVkDevice().createSemaphore({}));
         m_RenderFinishedSemaphores.emplace_back(m_Device->GetVkDevice().createSemaphore({}));
         m_InFlightFences.emplace_back(m_Device->GetVkDevice().createFence(ci));
      }
   }


   void VulkanWindowGC::DestroySyncObjects() {
      if (m_Device) {
         for (auto semaphore : m_ImageAvailableSemaphores) {
            m_Device->GetVkDevice().destroy(semaphore);
         }
         m_ImageAvailableSemaphores.clear();

         for (auto semaphore : m_RenderFinishedSemaphores) {
            m_Device->GetVkDevice().destroy(semaphore);
         }
         m_RenderFinishedSemaphores.clear();

         for (auto fence : m_InFlightFences) {
            m_Device->GetVkDevice().destroy(fence);
         }
         m_InFlightFences.clear();
         m_ImagesInFlight.clear();
      }
   }


   void VulkanWindowGC::RecreateSwapChain() {
      m_Device->GetVkDevice().waitIdle();
      DestroyImageViews();
      CreateSwapChain();
      CreateImageViews();

      DestroyDepthStencil();
      CreateDepthStencil();

      DestroyFrameBuffers();
      CreateFrameBuffers();

      m_WantResize = false;
   }

   void VulkanWindowGC::OnWindowResize(const WindowResizeEvent& event) {
      if (event.Sender == m_Window) {
         m_WantResize = true;
      }
   }

}
