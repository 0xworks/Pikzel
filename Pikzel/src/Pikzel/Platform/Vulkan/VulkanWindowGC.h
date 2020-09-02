#pragma once

#include "VulkanGraphicsContext.h"
#include "Pikzel/Events/WindowEvents.h"

namespace Pikzel {

   class VulkanWindowGC : public VulkanGraphicsContext {
   public:
      VulkanWindowGC(std::shared_ptr<VulkanDevice> device, GLFWwindow* window);
      virtual ~VulkanWindowGC();

      virtual void BeginFrame() override;
      virtual void EndFrame() override;

      virtual void SwapBuffers() override;

   private:
      void CreateSurface();
      void DestroySurface();

      vk::SurfaceFormatKHR SelectSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats);
      vk::PresentModeKHR SelectPresentMode(const std::vector<vk::PresentModeKHR>& availablePresentModes);

      void CreateSwapChain();
      void DestroySwapChain(vk::SwapchainKHR& swapChain);

      void CreateImageViews();
      void DestroyImageViews();

      vk::Extent2D SelectSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities);

      void CreateFrameBuffers();
      void DestroyFrameBuffers();

      void CreateSyncObjects();
      void DestroySyncObjects();

      void RecreateSwapChain();
      void OnWindowResize(const WindowResizeEvent& event);

   private:
      GLFWwindow* m_Window;     // VulkanWindowGC does not own the window!

      vk::SurfaceKHR m_Surface;

      vk::SwapchainKHR m_SwapChain;
      std::vector<VulkanImage> m_SwapChainImages;

      std::vector<vk::Framebuffer> m_SwapChainFrameBuffers;

      uint32_t m_MaxFramesInFlight = 2;
      uint32_t m_CurrentFrame = 0; // which frame (up to MaxFramesInFlight) are we currently rendering
      uint32_t m_CurrentImage = 0; // which swap chain image are we currently rendering to
      std::vector<vk::Semaphore> m_ImageAvailableSemaphores;
      std::vector<vk::Semaphore> m_RenderFinishedSemaphores;
      std::vector<vk::Fence> m_InFlightFences;
      std::vector<vk::Fence> m_ImagesInFlight;

      bool m_WantResize = false;

   };

}
