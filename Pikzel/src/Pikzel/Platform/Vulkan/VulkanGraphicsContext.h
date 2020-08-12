#pragma once

#include "Pikzel/Renderer/GraphicsContext.h"
#include "DescriptorBinding.h"
#include "Image.h"
#include "QueueFamilyIndices.h"

#include <vulkan/vulkan.hpp>

struct GLFWwindow;

namespace Pikzel {

   class VulkanGraphicsContext : public GraphicsContext {
   public:
      VulkanGraphicsContext(vk::Instance instance, GLFWwindow* window); // ownership of instance and window is not transferred, and VulkanGraphicsContext destructor does not destroy these
      virtual ~VulkanGraphicsContext();

      virtual void UploadImGuiFonts() override;

      virtual void BeginFrame() override;
      virtual void EndFrame() override;

      virtual void BeginImGuiFrame() override;
      virtual void EndImGuiFrame() override;

      virtual void SwapBuffers() override;

   private:
      void CreateSurface();
      void DestroySurface();

      QueueFamilyIndices FindQueueFamilies(vk::PhysicalDevice physicalDevice);
      vk::Format FindSupportedFormat(const std::vector<vk::Format>& candidates, vk::ImageTiling tiling, vk::FormatFeatureFlags features);

      bool IsPhysicalDeviceSuitable(vk::PhysicalDevice physicalDevice);
      std::vector<const char*> GetRequiredDeviceExtensions();
      vk::PhysicalDeviceFeatures GetRequiredPhysicalDeviceFeatures(vk::PhysicalDeviceFeatures availableFeatures);
      void* GetRequiredPhysicalDeviceFeaturesEXT();

      void SelectPhysicalDevice();

      vk::SurfaceFormatKHR SelectSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats);
      vk::PresentModeKHR SelectPresentMode(const std::vector<vk::PresentModeKHR>& availablePresentModes);

      void CreateDevice();
      void DestroyDevice();

      void CreateSwapChain();
      void DestroySwapChain(vk::SwapchainKHR& swapChain);

      void CreateImageViews();
      void DestroyImageViews();

      vk::Extent2D SelectSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities);

      void CreateDepthStencil();
      void DestroyDepthStencil();

      vk::RenderPass CreateRenderPass();  // need to CreateSwapChain() and CreateDepthStencil() first (because depends on m_Format and m_DepthFormat)
      void DestroyRenderPass(vk::RenderPass renderPass);

      void CreateFrameBuffers();
      void DestroyFrameBuffers();

      vk::DescriptorPool CreateDescriptorPool(const vk::ArrayProxy<DescriptorBinding>& descriptorBindings, size_t maxSets);
      void DestroyDescriptorPool(vk::DescriptorPool descriptorPool);

      void CreateCommandPool();
      void DestroyCommandPool();

      void CreateCommandBuffers();
      void DestroyCommandBuffers();

      void CreateSyncObjects();
      void DestroySyncObjects();

      void CreatePipelineCache();
      void DestroyPipelineCache();

      void SubmitSingleTimeCommands(const std::function<void(vk::CommandBuffer)>& action);

   private:
      vk::Instance m_Instance;  // VulkanGraphicsContext does not own the instance!
      GLFWwindow* m_Window;     // VulkanGraphicsContext does not own the window!
      vk::SurfaceKHR m_Surface;

      vk::PhysicalDevice m_PhysicalDevice;
      vk::PhysicalDeviceProperties m_PhysicalDeviceProperties;
      vk::PhysicalDeviceFeatures m_PhysicalDeviceFeatures;                 // features that are available on the selected physical device
      vk::PhysicalDeviceFeatures m_EnabledPhysicalDeviceFeatures;          // features that have been enabled
      vk::PhysicalDeviceMemoryProperties m_PhysicalDeviceMemoryProperties;
      QueueFamilyIndices m_QueueFamilyIndices;

      vk::Device m_Device;

      vk::Queue m_GraphicsQueue;
      vk::Queue m_PresentQueue;

      vk::Format m_Format = vk::Format::eUndefined;
      vk::Extent2D m_Extent;
      vk::SwapchainKHR m_SwapChain;
      std::vector<Image> m_SwapChainImages;
      bool m_WantResize = false;

      vk::Format m_DepthFormat;
      std::unique_ptr<Image> m_DepthImage;

      vk::RenderPass m_RenderPass;
      std::vector<vk::Framebuffer> m_SwapChainFrameBuffers;

      vk::RenderPass m_RenderPassImGui;
      vk::DescriptorPool m_DescriptorPoolImGui;

      vk::CommandPool m_CommandPool;
      std::vector<vk::CommandBuffer> m_CommandBuffers;

      uint32_t m_MaxFramesInFlight = 2;
      uint32_t m_CurrentFrame = 0; // which frame (up to MaxFramesInFlight) are we currently rendering
      uint32_t m_CurrentImage = 0; // which swap chain image are we currently rendering to
      std::vector<vk::Semaphore> m_ImageAvailableSemaphores;
      std::vector<vk::Semaphore> m_RenderFinishedSemaphores;
      std::vector<vk::Fence> m_InFlightFences;
      std::vector<vk::Fence> m_ImagesInFlight;

      vk::PipelineCache m_PipelineCache;

      bool m_ImGuiFrameStarted = false;

   private:
      static bool s_VulkanInitialized;
   };

}
