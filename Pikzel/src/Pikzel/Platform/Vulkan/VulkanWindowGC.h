#pragma once

#include "VulkanGraphicsContext.h"

#include "Pikzel/Core/Window.h"
#include "Pikzel/Events/WindowEvents.h"

#include <array>

namespace Pikzel {

   class VulkanWindowGC : public VulkanGraphicsContext {
   public:
      VulkanWindowGC(std::shared_ptr<VulkanDevice> device, const Window& window);
      virtual ~VulkanWindowGC();

      virtual void BeginFrame() override;
      virtual void EndFrame() override;

      virtual void SwapBuffers() override;

      void Bind(const VertexBuffer& buffer) override;
      void Unbind(const VertexBuffer& buffer) override;

      void Bind(const IndexBuffer& buffer) override;
      void Unbind(const IndexBuffer& buffer) override;

      void Bind(const Texture2D& texture, uint32_t slot) override;
      void Unbind(const Texture2D& texture) override;

      void Bind(const Pipeline& pipeline) override;
      void Unbind(const Pipeline& pipeline) override;

      std::unique_ptr<Pipeline> CreatePipeline(const PipelineSettings& settings) override;

      void DrawIndexed(VertexBuffer& vertexBuffer, IndexBuffer& indexBuffer, uint32_t indexCount = 0) override;

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
      std::array<vk::ClearValue, 2> m_ClearValues;

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
