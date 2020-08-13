#include "vkpch.h"
#include "VulkanGraphicsContext.h"
#include "SwapChainSupportDetails.h"

#include "Pikzel/Events/EventDispatcher.h"

#include <GLFW/glfw3.h>

#include <imgui.h>
#include <imgui_internal.h>
#include <examples/imgui_impl_glfw.h>
#include <examples/imgui_impl_vulkan.h>

#include <algorithm>
#include <set>

namespace Pikzel {

   void ImGuiVulkanResultCallback(const VkResult err) {
      if (err != VK_SUCCESS) {
         throw std::runtime_error("ImGui Vulkan error!");
      }
   }


   VulkanGraphicsContext::VulkanGraphicsContext(vk::Instance instance, GLFWwindow* window)
      : m_Instance(instance)
      , m_Window(window) {

      CreateSurface();
      SelectPhysicalDevice();
      CreateDevice();
      CreateSwapChain();
      CreateImageViews();
      CreateDepthStencil();
      m_RenderPass = CreateRenderPass();
      m_DescriptorPoolImGui = CreateDescriptorPool(DescriptorBinding {0, 1, vk::DescriptorType::eCombinedImageSampler, {}}, 1);
      m_RenderPassImGui = CreateRenderPass();
      CreateFrameBuffers();
      CreateCommandPool();
      CreateCommandBuffers();
      CreateSyncObjects();
      CreatePipelineCache();

      EventDispatcher::Connect<WindowResizeEvent, &VulkanGraphicsContext::OnWindowResize>(*this);

      IMGUI_CHECKVERSION();
      ImGui::CreateContext();

      ImGuiIO& io = ImGui::GetIO();
      io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
      io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
      io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

      ImGui_ImplGlfw_InitForVulkan(m_Window, true);
      ImGui_ImplVulkan_InitInfo init_info = {};
      init_info.Instance = m_Instance;
      init_info.PhysicalDevice = m_PhysicalDevice;
      init_info.Device = m_Device;
      init_info.QueueFamily = m_QueueFamilyIndices.GraphicsFamily.value();
      init_info.Queue = m_GraphicsQueue;
      init_info.PipelineCache = m_PipelineCache;
      init_info.DescriptorPool = m_DescriptorPoolImGui;
      init_info.Allocator = nullptr; // TODO: proper allocator...
      init_info.MinImageCount = static_cast<uint32_t>(m_SwapChainImages.size());
      init_info.ImageCount = static_cast<uint32_t>(m_SwapChainImages.size());
      init_info.CheckVkResultFn = ImGuiVulkanResultCallback;
      ImGui_ImplVulkan_Init(&init_info, m_RenderPassImGui);
   }


   VulkanGraphicsContext::~VulkanGraphicsContext() {
      if (ImGui::GetCurrentContext()) {
         ImGui_ImplVulkan_Shutdown();
         ImGui_ImplGlfw_Shutdown();
         ImGui::DestroyContext();
      }
      DestroyPipelineCache();
      DestroySyncObjects();
      DestroyCommandBuffers();
      DestroyCommandPool();
      DestroyFrameBuffers();
      DestroyRenderPass(m_RenderPassImGui);
      DestroyDescriptorPool(m_DescriptorPoolImGui);
      DestroyRenderPass(m_RenderPass);
      DestroyDepthStencil();
      DestroyImageViews();
      DestroySwapChain(m_SwapChain);
      DestroyDevice();
      DestroySurface();
   }


   void VulkanGraphicsContext::UploadImGuiFonts() {
      SubmitSingleTimeCommands([] (vk::CommandBuffer commandBuffer) {
         if (!ImGui_ImplVulkan_CreateFontsTexture(commandBuffer)) {
            throw std::runtime_error("failed to create ImGui font textures!");
         }
      });
      ImGui_ImplVulkan_DestroyFontUploadObjects();
   }


   void VulkanGraphicsContext::BeginFrame() {
      m_Device.waitForFences(m_InFlightFences[m_CurrentFrame], true, UINT64_MAX);
      auto rv = m_Device.acquireNextImageKHR(m_SwapChain, UINT64_MAX, m_ImageAvailableSemaphores[m_CurrentFrame], nullptr);

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
         m_Device.waitForFences(m_ImagesInFlight[m_CurrentImage], true, UINT64_MAX);
      }
      // Mark the image as now being in use by this frame
      m_ImagesInFlight[m_CurrentImage] = m_InFlightFences[m_CurrentFrame];

      vk::CommandBufferBeginInfo commandBufferBI = {
         vk::CommandBufferUsageFlagBits::eSimultaneousUse
      };
      m_CommandBuffers[m_CurrentImage].begin(commandBufferBI);

      // TODO: you probably need to begin render pass here...
      //       theres a bit of a question around how will client app do multiple render passes?

   }


   void VulkanGraphicsContext::EndFrame() {
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

      m_Device.resetFences(m_InFlightFences[m_CurrentFrame]);
      m_GraphicsQueue.submit(si, m_InFlightFences[m_CurrentFrame]);

      if (m_ImGuiFrameStarted) {
         if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
         }
         m_ImGuiFrameStarted = false;
      }
   }


   void VulkanGraphicsContext::BeginImGuiFrame() {
      ImGui_ImplVulkan_NewFrame();
      ImGui_ImplGlfw_NewFrame();
      ImGui::NewFrame();
      m_ImGuiFrameStarted = true;
   }


   void VulkanGraphicsContext::EndImGuiFrame() {
      std::array<vk::ClearValue, 2> clearValues = {
         vk::ClearColorValue {std::array<float,4>{0.0f, 0.0f, 0.0f, 1.0f}},
         vk::ClearDepthStencilValue {1.0f, 0}
      };

      vk::RenderPassBeginInfo renderPassBI = {
         m_RenderPassImGui                          /*renderPass*/,
         m_SwapChainFrameBuffers[m_CurrentImage]    /*framebuffer*/,
         { {0,0}, m_Extent }                        /*renderArea*/,
         2                                          /*clearValueCount*/,
         clearValues.data()                         /*pClearValues*/
      };

      vk::CommandBuffer commandBuffer = m_CommandBuffers[m_CurrentImage];
      commandBuffer.beginRenderPass(renderPassBI, vk::SubpassContents::eInline);
      ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);
      commandBuffer.endRenderPass();
   }


   void VulkanGraphicsContext::SwapBuffers() {
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
      auto result = vkQueuePresentKHR(m_PresentQueue, &(VkPresentInfoKHR)pi);
      if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || m_WantResize) {
         RecreateSwapChain();
      } else if (result != VK_SUCCESS) {
         throw std::runtime_error("Failed to present swap chain image!");
      }

      m_CurrentFrame = ++m_CurrentFrame % m_MaxFramesInFlight;
   }


   void VulkanGraphicsContext::CreateSurface() {
      VkSurfaceKHR surface;
      if (glfwCreateWindowSurface(m_Instance, m_Window, nullptr, &surface) != VK_SUCCESS) {
         throw std::runtime_error("failed to create window surface!");
      }
      m_Surface = surface;
   }


   void VulkanGraphicsContext::DestroySurface() {
      if (m_Instance && m_Surface) {
         m_Instance.destroy(m_Surface);
         m_Surface = nullptr;
      }
   }


   QueueFamilyIndices VulkanGraphicsContext::FindQueueFamilies(vk::PhysicalDevice physicalDevice) {
      QueueFamilyIndices indices;

      std::vector<vk::QueueFamilyProperties> queueFamilies = physicalDevice.getQueueFamilyProperties();
      int i = 0;
      for (const auto& queueFamily : queueFamilies) {
         if (queueFamily.queueFlags & vk::QueueFlagBits::eGraphics) {
            indices.GraphicsFamily = i;
         }

         if (physicalDevice.getSurfaceSupportKHR(i, m_Surface)) {
            indices.PresentFamily = i;
         }

         if (indices.IsComplete()) {
            break;
         }

         ++i;
      }

      return indices;
   }


   vk::Format VulkanGraphicsContext::FindSupportedFormat(const std::vector<vk::Format>& candidates, vk::ImageTiling tiling, vk::FormatFeatureFlags features) {
      for (auto format : candidates) {
         vk::FormatProperties props = m_PhysicalDevice.getFormatProperties(format);
         if ((tiling == vk::ImageTiling::eLinear) && ((props.linearTilingFeatures & features) == features)) {
            return format;
         } else if ((tiling == vk::ImageTiling::eOptimal) && ((props.optimalTilingFeatures & features) == features)) {
            return format;
         }
      }
      throw std::runtime_error("failed to find supported format!");
   }


   bool CheckDeviceExtensionSupport(vk::PhysicalDevice physicalDevice, std::vector<const char*> extensions) {
      bool allExtensionsFound = true;
      std::vector<vk::ExtensionProperties> availableExtensions = physicalDevice.enumerateDeviceExtensionProperties();
      for (const auto& extension : extensions) {
         bool extensionFound = false;
         for (const auto& availableExtension : availableExtensions) {
            if (strcmp(extension, availableExtension.extensionName) == 0) {
               extensionFound = true;
               break;
            }
         }
         if (!extensionFound) {
            allExtensionsFound = false;
            break;
         }
      }
      return allExtensionsFound;
   }


   SwapChainSupportDetails QuerySwapChainSupport(vk::PhysicalDevice device, vk::SurfaceKHR surface) {
      return {
         device.getSurfaceCapabilitiesKHR(surface),
         device.getSurfaceFormatsKHR(surface),
         device.getSurfacePresentModesKHR(surface)
      };
   }


   bool VulkanGraphicsContext::IsPhysicalDeviceSuitable(vk::PhysicalDevice physicalDevice) {
      bool extensionsSupported = false;
      bool swapChainAdequate = false;
      QueueFamilyIndices indices = FindQueueFamilies(physicalDevice);
      if (indices.IsComplete()) {
         extensionsSupported = CheckDeviceExtensionSupport(physicalDevice, GetRequiredDeviceExtensions());
         if (extensionsSupported) {
            SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(physicalDevice, m_Surface);
            swapChainAdequate = !swapChainSupport.Formats.empty() && !swapChainSupport.PresentModes.empty();
         }
      }
      return indices.IsComplete() && extensionsSupported && swapChainAdequate;
   }


   std::vector<const char*> VulkanGraphicsContext::GetRequiredDeviceExtensions() {
      return {}; // no device extensions required  (yet...)
   }


   vk::PhysicalDeviceFeatures VulkanGraphicsContext::GetRequiredPhysicalDeviceFeatures(vk::PhysicalDeviceFeatures) {
      return {}; // no required device features (yet...)
   }


   void* VulkanGraphicsContext::GetRequiredPhysicalDeviceFeaturesEXT() {
      return nullptr;
   }


   void VulkanGraphicsContext::SelectPhysicalDevice() {
      uint32_t deviceCount = 0;
      std::vector<vk::PhysicalDevice> physicalDevices = m_Instance.enumeratePhysicalDevices();
      for (const auto& physicalDevice : physicalDevices) {
         if (IsPhysicalDeviceSuitable(physicalDevice)) {
            m_PhysicalDevice = physicalDevice;
            m_PhysicalDeviceProperties = m_PhysicalDevice.getProperties();
            m_PhysicalDeviceFeatures = m_PhysicalDevice.getFeatures();
            m_PhysicalDeviceMemoryProperties = m_PhysicalDevice.getMemoryProperties();
            m_QueueFamilyIndices = FindQueueFamilies(m_PhysicalDevice);
            m_DepthFormat = FindSupportedFormat(
               {vk::Format::eD32SfloatS8Uint, vk::Format::eD32Sfloat, vk::Format::eD24UnormS8Uint, vk::Format::eD16UnormS8Uint, vk::Format::eD16Unorm},
               vk::ImageTiling::eOptimal,
               vk::FormatFeatureFlagBits::eDepthStencilAttachment
            );
            break;
         }
      }
      if (!m_PhysicalDevice) {
         throw std::runtime_error("failed to find a suitable GPU!");
      }
   }


   vk::SurfaceFormatKHR VulkanGraphicsContext::SelectSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats) {
      for (const auto& availableFormat : availableFormats) {
         if (availableFormat.format == vk::Format::eB8G8R8A8Unorm && availableFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
            return availableFormat;
         }
      }
      return availableFormats[0];
   }


   vk::PresentModeKHR VulkanGraphicsContext::SelectPresentMode(const std::vector<vk::PresentModeKHR>& availablePresentModes) {
      for (const auto& availablePresentMode : availablePresentModes) {
         if (availablePresentMode == vk::PresentModeKHR::eMailbox) {
            return availablePresentMode;
         }
      }
      return vk::PresentModeKHR::eFifo;
   }


   void VulkanGraphicsContext::CreateDevice() {
      float queuePriority = 1.0f;

      std::vector<vk::DeviceQueueCreateInfo> deviceQueueCIs;
      std::set<uint32_t> uniqueQueueFamilies = {m_QueueFamilyIndices.GraphicsFamily.value(), m_QueueFamilyIndices.PresentFamily.value()};

      for (uint32_t queueFamily : uniqueQueueFamilies) {
         deviceQueueCIs.emplace_back(
            vk::DeviceQueueCreateFlags {},
            queueFamily,
            1,
            &queuePriority
         );
      }

      std::vector<const char*> deviceExtensions = GetRequiredDeviceExtensions();

      // We always need swap chain extension
      deviceExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

      m_EnabledPhysicalDeviceFeatures = GetRequiredPhysicalDeviceFeatures(m_PhysicalDeviceFeatures);

      vk::DeviceCreateInfo ci = {
         {},
         static_cast<uint32_t>(deviceQueueCIs.size())     /*queueCreateInfoCount*/,
         deviceQueueCIs.data()                            /*pQueueCreateInfos*/,
         0                                                /*enabledLayerCount*/,
         nullptr                                          /*ppEnabledLayerNames*/,
         static_cast<uint32_t>(deviceExtensions.size())   /*enabledExtensionCount*/,
         deviceExtensions.data()                          /*ppEnabledExtensionNames*/,
         &m_EnabledPhysicalDeviceFeatures                 /*pEnabledFeatures*/
      };
      ci.pNext = GetRequiredPhysicalDeviceFeaturesEXT();

#ifdef PKZL_DEBUG
      std::vector<const char*> layers = {"VK_LAYER_KHRONOS_validation"};
      ci.enabledLayerCount = static_cast<uint32_t>(layers.size());
      ci.ppEnabledLayerNames = layers.data();
#endif

      m_Device = m_PhysicalDevice.createDevice(ci);

      m_GraphicsQueue = m_Device.getQueue(m_QueueFamilyIndices.GraphicsFamily.value(), 0);
      m_PresentQueue = m_Device.getQueue(m_QueueFamilyIndices.PresentFamily.value(), 0);
   }


   void VulkanGraphicsContext::DestroyDevice() {
      if (m_Device) {
         m_Device.destroy();
         m_Device = nullptr;
      }
   }


   void VulkanGraphicsContext::CreateSwapChain() {
      vk::SwapchainKHR oldSwapChain = m_SwapChain;

      SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(m_PhysicalDevice, m_Surface);

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

      uint32_t queueFamilyIndices[] = {m_QueueFamilyIndices.GraphicsFamily.value(), m_QueueFamilyIndices.PresentFamily.value()};
      if (m_QueueFamilyIndices.GraphicsFamily != m_QueueFamilyIndices.PresentFamily) {
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

      m_SwapChain = m_Device.createSwapchainKHR(ci);
      DestroySwapChain(oldSwapChain);
      std::vector<vk::Image> swapChainImages = m_Device.getSwapchainImagesKHR(m_SwapChain);
      for (const auto& image : swapChainImages) {
         m_SwapChainImages.emplace_back(m_Device, image);
      }
   }


   void VulkanGraphicsContext::DestroySwapChain(vk::SwapchainKHR& swapChain) {
      if (m_Device && swapChain) {
         m_SwapChainImages.clear();
         m_Device.destroy(swapChain);
         swapChain = nullptr;
      }
   }


   void VulkanGraphicsContext::CreateImageViews() {
      for (auto& image : m_SwapChainImages) {
         image.CreateImageView(m_Format, vk::ImageAspectFlagBits::eColor, 1);
      }
   }


   void VulkanGraphicsContext::DestroyImageViews() {
      if (m_Device) {
         for (auto& image : m_SwapChainImages) {
            image.DestroyImageView();
         }
      }
   }


   vk::Extent2D VulkanGraphicsContext::SelectSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities) {
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


   void VulkanGraphicsContext::CreateDepthStencil() {
      // TODO anti-aliasing
      m_DepthImage = std::make_unique<Image>(m_Device, m_PhysicalDevice, m_Extent.width, m_Extent.height, 1, vk::SampleCountFlagBits::e1, m_DepthFormat, vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eDepthStencilAttachment, vk::MemoryPropertyFlagBits::eDeviceLocal);
      m_DepthImage->CreateImageView(m_DepthFormat, vk::ImageAspectFlagBits::eDepth, 1);
   }


   void VulkanGraphicsContext::DestroyDepthStencil() {
      m_DepthImage.reset(nullptr);
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

      return m_Device.createRenderPass({
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
            m_Device.destroy(renderPass);
            renderPass = nullptr;
         }
      }
   }


   void VulkanGraphicsContext::CreateFrameBuffers() {
      std::array<vk::ImageView, 2> attachments = {
         nullptr,
         m_DepthImage->m_ImageView
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
         attachments[0] = swapChainImage.m_ImageView;
         m_SwapChainFrameBuffers.push_back(m_Device.createFramebuffer(ci));
      }
   }


   void VulkanGraphicsContext::DestroyFrameBuffers() {
      if (m_Device) {
         for (auto frameBuffer : m_SwapChainFrameBuffers) {
            m_Device.destroy(frameBuffer);
         }
         m_SwapChainFrameBuffers.clear();
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
      return m_Device.createDescriptorPool(descriptorPoolCI);
   }


   void VulkanGraphicsContext::DestroyDescriptorPool(vk::DescriptorPool descriptorPool) {
      if (m_Device) {
         if (descriptorPool) {
            m_Device.destroy(descriptorPool);
            descriptorPool = nullptr;
         }
      }
   }


   void VulkanGraphicsContext::CreateCommandPool() {
      m_CommandPool = m_Device.createCommandPool({
         {vk::CommandPoolCreateFlagBits::eResetCommandBuffer},
         m_QueueFamilyIndices.GraphicsFamily.value()
      });
   }


   void VulkanGraphicsContext::DestroyCommandPool() {
      if (m_Device && m_CommandPool) {
         m_Device.destroy(m_CommandPool);
         m_CommandPool = nullptr;
      }
   }


   void VulkanGraphicsContext::CreateCommandBuffers() {
      m_CommandBuffers = m_Device.allocateCommandBuffers({
         m_CommandPool                                           /*commandPool*/,
         vk::CommandBufferLevel::ePrimary                        /*level*/,
         static_cast<uint32_t>(m_SwapChainFrameBuffers.size())   /*commandBufferCount*/
      });
   }


   void VulkanGraphicsContext::DestroyCommandBuffers() {
      if (m_Device && m_CommandPool) {
         m_Device.freeCommandBuffers(m_CommandPool, m_CommandBuffers);
         m_CommandBuffers.clear();
      }
   }


   void VulkanGraphicsContext::CreateSyncObjects() {
      m_ImageAvailableSemaphores.reserve(m_MaxFramesInFlight);
      m_RenderFinishedSemaphores.reserve(m_MaxFramesInFlight);
      m_InFlightFences.reserve(m_MaxFramesInFlight);
      m_ImagesInFlight.resize(m_SwapChainImages.size(), nullptr);

      vk::FenceCreateInfo ci = {
         {vk::FenceCreateFlagBits::eSignaled}
      };

      for (uint32_t i = 0; i < m_MaxFramesInFlight; ++i) {
         m_ImageAvailableSemaphores.emplace_back(m_Device.createSemaphore({}));
         m_RenderFinishedSemaphores.emplace_back(m_Device.createSemaphore({}));
         m_InFlightFences.emplace_back(m_Device.createFence(ci));
      }
   }


   void VulkanGraphicsContext::DestroySyncObjects() {
      if (m_Device) {
         for (auto semaphore : m_ImageAvailableSemaphores) {
            m_Device.destroy(semaphore);
         }
         m_ImageAvailableSemaphores.clear();

         for (auto semaphore : m_RenderFinishedSemaphores) {
            m_Device.destroy(semaphore);
         }
         m_RenderFinishedSemaphores.clear();

         for (auto fence : m_InFlightFences) {
            m_Device.destroy(fence);
         }
         m_InFlightFences.clear();
         m_ImagesInFlight.clear();
      }
   }


   void VulkanGraphicsContext::CreatePipelineCache() {
      m_PipelineCache = m_Device.createPipelineCache({});
   }


   void VulkanGraphicsContext::DestroyPipelineCache() {
      if (m_Device && m_PipelineCache) {
         m_Device.destroy(m_PipelineCache);
      }
   }


   void VulkanGraphicsContext::SubmitSingleTimeCommands(const std::function<void(vk::CommandBuffer)>& action) {
      std::vector<vk::CommandBuffer> commandBuffers = m_Device.allocateCommandBuffers({
         m_CommandPool                    /*commandPool*/,
         vk::CommandBufferLevel::ePrimary /*level*/,
         1                                /*commandBufferCount*/
      });

      commandBuffers[0].begin({vk::CommandBufferUsageFlagBits::eOneTimeSubmit});
      action(commandBuffers[0]);
      commandBuffers[0].end();

      vk::SubmitInfo si;
      si.commandBufferCount = 1;
      si.pCommandBuffers = commandBuffers.data();
      m_GraphicsQueue.submit(si, nullptr);
      m_GraphicsQueue.waitIdle();
      m_Device.freeCommandBuffers(m_CommandPool, commandBuffers);
   }


   void VulkanGraphicsContext::RecreateSwapChain() {
      m_Device.waitIdle();
      DestroyImageViews();
      CreateSwapChain();
      CreateImageViews();

      DestroyDepthStencil();
      CreateDepthStencil();

      DestroyFrameBuffers();
      CreateFrameBuffers();

      // TODO: Do we need to do anything for ImGui overlay?

      // I don't _think_ we need to recreate command buffers here, as we are re-recording them each frame anyway (so they should get re-bound to newly create frame buffers)
      //DestroyCommandBuffers();
      //CreateCommandBuffers();

      // TODO: You may need to add other stuff here too, like DescriptorSets (?)

      m_WantResize = false;
   }

   void VulkanGraphicsContext::OnWindowResize(const WindowResizeEvent& event) {
      if (event.Sender == m_Window) {
         m_WantResize = true;
      }
   }
}