#pragma once

#include "VulkanGraphicsContext.h"

#include "Pikzel/Core/Window.h"
#include "Pikzel/Events/WindowEvents.h"

#include <array>

namespace Pikzel {

   class VulkanPipeline;

   class VulkanWindowGC : public VulkanGraphicsContext {
   public:
      VulkanWindowGC(std::shared_ptr<VulkanDevice> device, const Window& window);
      virtual ~VulkanWindowGC();

      virtual void BeginFrame() override;
      virtual void EndFrame() override;

      virtual void SwapBuffers() override;

      virtual void Bind(const VertexBuffer& buffer) override;
      virtual void Unbind(const VertexBuffer& buffer) override;

      virtual void Bind(const IndexBuffer& buffer) override;
      virtual void Unbind(const IndexBuffer& buffer) override;

      virtual void Bind(const UniformBuffer& buffer, entt::id_type resourceId) override;
      virtual void Unbind(const UniformBuffer& buffer) override;

      virtual void Bind(const Texture2D& texture, entt::id_type resourceId) override;
      virtual void Unbind(const Texture2D& texture) override;

      virtual void Bind(const Pipeline& pipeline) override;
      virtual void Unbind(const Pipeline& pipeline) override;

      virtual std::unique_ptr<Pipeline> CreatePipeline(const PipelineSettings& settings) override;

      virtual void PushConstant(const entt::id_type id, bool value) override;
      virtual void PushConstant(const entt::id_type id, int value) override;
      virtual void PushConstant(const entt::id_type id, uint32_t value) override;
      virtual void PushConstant(const entt::id_type id, float value) override;
      virtual void PushConstant(const entt::id_type id, double value) override;
      virtual void PushConstant(const entt::id_type id, const glm::bvec2& value) override;
      virtual void PushConstant(const entt::id_type id, const glm::bvec3& value) override;
      virtual void PushConstant(const entt::id_type id, const glm::bvec4& value) override;
      virtual void PushConstant(const entt::id_type id, const glm::ivec2& value) override;
      virtual void PushConstant(const entt::id_type id, const glm::ivec3& value) override;
      virtual void PushConstant(const entt::id_type id, const glm::ivec4& value) override;
      virtual void PushConstant(const entt::id_type id, const glm::uvec2& value) override;
      virtual void PushConstant(const entt::id_type id, const glm::uvec3& value) override;
      virtual void PushConstant(const entt::id_type id, const glm::uvec4& value) override;
      virtual void PushConstant(const entt::id_type id, const glm::vec2& value) override;
      virtual void PushConstant(const entt::id_type id, const glm::vec3& value) override;
      virtual void PushConstant(const entt::id_type id, const glm::vec4& value) override;
      virtual void PushConstant(const entt::id_type id, const glm::dvec2& value) override;
      virtual void PushConstant(const entt::id_type id, const glm::dvec3& value) override;
      virtual void PushConstant(const entt::id_type id, const glm::dvec4& value) override;
      virtual void PushConstant(const entt::id_type id, const glm::mat2& value) override;
      //virtual void PushConstant(const entt::id_type id, const glm::mat2x3& value) override;
      virtual void PushConstant(const entt::id_type id, const glm::mat2x4& value) override;
      virtual void PushConstant(const entt::id_type id, const glm::mat3x2& value) override;
      //virtual void PushConstant(const entt::id_type id, const glm::mat3& value) override;
      virtual void PushConstant(const entt::id_type id, const glm::mat3x4& value) override;
      virtual void PushConstant(const entt::id_type id, const glm::mat4x2& value) override;
      //virtual void PushConstant(const entt::id_type id, const glm::mat4x3& value) override;
      virtual void PushConstant(const entt::id_type id, const glm::mat4& value) override;
      virtual void PushConstant(const entt::id_type id, const glm::dmat2& value) override;
      //virtual void PushConstant(const entt::id_type id, const glm::dmat2x3& value) override;
      virtual void PushConstant(const entt::id_type id, const glm::dmat2x4& value) override;
      virtual void PushConstant(const entt::id_type id, const glm::dmat3x2& value) override;
      //virtual void PushConstant(const entt::id_type id, const glm::dmat3& value) override;
      virtual void PushConstant(const entt::id_type id, const glm::dmat3x4& value) override;
      virtual void PushConstant(const entt::id_type id, const glm::dmat4x2& value) override;
      //virtual void PushConstant(const entt::id_type id, const glm::dmat4x3& value) override;
      virtual void PushConstant(const entt::id_type id, const glm::dmat4& value) override;

      virtual void DrawIndexed(const VertexBuffer& vertexBuffer, const IndexBuffer& indexBuffer, uint32_t indexCount = 0) override;

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

      GLFWwindow* m_Window = nullptr;             // VulkanWindowGC does not own the window!
      const VulkanPipeline* m_Pipeline = nullptr; // currently bound pipeline

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
