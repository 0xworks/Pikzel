#pragma once

#include "DescriptorBinding.h"
#include "VulkanDevice.h"
#include "VulkanFence.h"
#include "VulkanFramebuffer.h"
#include "VulkanImage.h"

#include "Pikzel/Core/Window.h"
#include "Pikzel/Events/WindowEvents.h"
#include "Pikzel/Renderer/GraphicsContext.h"

#include <array>

namespace Pikzel {

   class VulkanPipeline;

   class VulkanGraphicsContext : public GraphicsContext {
   protected:
      VulkanGraphicsContext(std::shared_ptr<VulkanDevice> device);
      virtual ~VulkanGraphicsContext();

   public:
      virtual void InitializeImGui() override;
      virtual ImGuiContext* GetImGuiContext() override;
      virtual void BeginImGuiFrame() override;
      virtual void EndImGuiFrame() override;

      virtual void Bind(const VertexBuffer& buffer) override;
      virtual void Unbind(const VertexBuffer& buffer) override;

      virtual void Bind(const IndexBuffer& buffer) override;
      virtual void Unbind(const IndexBuffer& buffer) override;

      virtual void Bind(const UniformBuffer& buffer, const entt::id_type resourceId) override;
      virtual void Unbind(const UniformBuffer& buffer) override;

      virtual void Bind(const Texture& texture, const entt::id_type resourceId) override;
      virtual void Unbind(const Texture& texture) override;

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

      virtual void DrawTriangles(const VertexBuffer& vertexBuffer, const uint32_t vertexCount, const uint32_t vertexOffset = 0) override;
      virtual void DrawIndexed(const VertexBuffer& vertexBuffer, const IndexBuffer& indexBuffer, const uint32_t indexCount = 0, const uint32_t vertexOffset = 0) override;

   public:
      vk::RenderPass GetVkRenderPass(BeginFrameOp operation) const;
      vk::PipelineCache GetVkPipelineCache() const;

      virtual vk::CommandBuffer GetVkCommandBuffer() = 0;
      virtual std::shared_ptr<VulkanFence> GetFence() = 0;

      vk::SampleCountFlagBits GetNumSamples() const;

   protected:
      vk::RenderPass CreateRenderPass(const std::vector<vk::AttachmentDescription2>& attachments);
      void DestroyRenderPass(vk::RenderPass renderPass);

      vk::DescriptorPool CreateDescriptorPool(const vk::ArrayProxy<const DescriptorBinding>& descriptorBindings, size_t maxSets);
      void DestroyDescriptorPool(vk::DescriptorPool descriptorPool);

      void CreateCommandPool();
      void DestroyCommandPool();

      void CreateCommandBuffers(const uint32_t commandBufferCount);
      void DestroyCommandBuffers();

      void CreatePipelineCache();
      void DestroyPipelineCache();

      void BindDescriptorSets();
      void UnbindDescriptorSets();

   protected:
      std::shared_ptr<VulkanDevice> m_Device;

      vk::Extent2D m_Extent;

      vk::SampleCountFlagBits m_SampleCount = vk::SampleCountFlagBits::e1;
      std::unique_ptr<VulkanImage> m_ColorImage;

      std::unique_ptr<VulkanImage> m_DepthImage;

      // We need a different render pass for each possible "BeginFrameOp".
      // This is less that ideal!
      std::unordered_map<BeginFrameOp, vk::RenderPass> m_RenderPasses;

      vk::CommandPool m_CommandPool;
      std::vector<vk::CommandBuffer> m_CommandBuffers;

      vk::PipelineCache m_PipelineCache;
      VulkanPipeline* m_Pipeline = nullptr;       // currently bound pipeline  (TODO: should be a shared_ptr?)
   };


   class VulkanWindowGC : public VulkanGraphicsContext {
   public:
      VulkanWindowGC(std::shared_ptr<VulkanDevice> device, const Window& window);
      virtual ~VulkanWindowGC();

      virtual void BeginFrame(const BeginFrameOp operation = BeginFrameOp::ClearAll) override;
      virtual void EndFrame() override;

      virtual void InitializeImGui() override;
      virtual void BeginImGuiFrame() override;
      virtual void EndImGuiFrame() override;

      virtual void Bind(const Pipeline& pipeline) override;
      virtual void Unbind(const Pipeline& pipeline) override;

      virtual void SwapBuffers() override;

   public:
      virtual vk::CommandBuffer GetVkCommandBuffer() override;
      virtual std::shared_ptr<VulkanFence> GetFence() override;

   private:
      void CreateSurface();
      void DestroySurface();

      vk::SurfaceFormatKHR SelectSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats);
      vk::PresentModeKHR SelectPresentMode(const std::vector<vk::PresentModeKHR>& availablePresentModes);

      void CreateSwapChain();
      void DestroySwapChain(vk::SwapchainKHR& swapChain);

      void CreateColorImage();
      void DestroyColorImage();

      void CreateDepthStencil();
      void DestroyDepthStencil();

      void CreateImageViews();
      void DestroyImageViews();

      vk::Extent2D SelectSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities);

      void CreateFramebuffers();
      void DestroyFramebuffers();

      void CreateSyncObjects();
      void DestroySyncObjects();

      void RecreateSwapChain();

   private:
      void OnWindowResize(const WindowResizeEvent& event);
      void OnWindowVSyncChanged(const WindowVSyncChangedEvent& event);

   private:
      std::array<vk::ClearValue, 2> m_ClearValues;
      GLFWwindow* m_Window = nullptr;             // VulkanWindowGC does not own the window!

      vk::Format m_Format = vk::Format::eUndefined;
      vk::Format m_DepthFormat = vk::Format::eUndefined;

      vk::SurfaceKHR m_Surface;

      vk::SwapchainKHR m_SwapChain;
      std::vector<VulkanImage> m_SwapChainImages;

      vk::DescriptorPool m_DescriptorPoolImGui;
      vk::RenderPass m_RenderPassImGui;

      std::vector<vk::Framebuffer> m_SwapChainFramebuffers;

      uint32_t m_MaxFramesInFlight = 2;
      uint32_t m_CurrentFrame = 0; // which frame (up to MaxFramesInFlight) are we currently rendering
      uint32_t m_CurrentImage = 0; // which swap chain image are we currently rendering to
      std::vector<vk::Semaphore> m_ImageAvailableSemaphores;
      std::vector<vk::Semaphore> m_RenderFinishedSemaphores;
      std::vector<std::shared_ptr<VulkanFence>> m_InFlightFences;

      bool m_IsVSync = false;
      bool m_WantResize = false;
      bool m_ImGuiFrameStarted = false;
   };


   class VulkanFramebufferGC : public VulkanGraphicsContext {
   public:
      VulkanFramebufferGC(std::shared_ptr<VulkanDevice> device, VulkanFramebuffer* framebuffer); // raw pointer is fine here.  We know the VulkanFramebufferGC lifetime is nested inside the framebuffer's lifetime
      virtual ~VulkanFramebufferGC();

      virtual void BeginFrame(const BeginFrameOp operation = BeginFrameOp::ClearAll) override;
      virtual void EndFrame() override;

      virtual void Bind(const Pipeline& pipeline) override;
      virtual void Unbind(const Pipeline& pipeline) override;

      virtual void SwapBuffers() override;

   public:
      virtual vk::CommandBuffer GetVkCommandBuffer() override;
      virtual std::shared_ptr<VulkanFence> GetFence() override;

   private:
      void CreateSyncObjects();
      void DestroySyncObjects();

   private:
      std::vector<vk::ClearValue> m_ClearValues;
      vk::Extent2D m_Extent;
      VulkanFramebuffer* m_Framebuffer;
      std::shared_ptr<VulkanFence> m_InFlightFence;
   };

}
