#include "vkpch.h"
#include "VulkanWindowGC.h"

#include "SwapChainSupportDetails.h"
#include "VulkanBuffer.h"
#include "VulkanPipeline.h"
#include "VulkanUtility.h"

#include "Pikzel/Events/EventDispatcher.h"

namespace Pikzel {

   VulkanWindowGC::VulkanWindowGC(std::shared_ptr<VulkanDevice> device, const Window& window)
   : VulkanGraphicsContext {device}
   , m_Window {static_cast<GLFWwindow*>(window.GetNativeWindow())}
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

      // Set clear values for all framebuffer attachments with loadOp set to clear
      // We use two attachments (color and depth) that are cleared at the start of the subpass and as such we need to set clear values for both
      glm::vec4 clearColor = window.GetClearColor();
      m_ClearValues = {
         vk::ClearColorValue {std::array<float, 4>{clearColor.r, clearColor.g, clearColor.b, clearColor.a}},
         vk::ClearDepthStencilValue {1.0f, 0}
      };

   }


   VulkanWindowGC::~VulkanWindowGC() {
      m_Device->GetVkDevice().waitIdle();
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

      // TODO: Not sure that this is the best place to begin render pass.
      //       What if you need/want multiple render passes?  How will the client control this?

      vk::RenderPassBeginInfo renderPassBI = {
         m_RenderPass                                 /*renderPass*/,
         m_SwapChainFrameBuffers[m_CurrentImage]      /*framebuffer*/,
         { {0,0}, m_Extent }                          /*renderArea*/,
         static_cast<uint32_t>(m_ClearValues.size())  /*clearValueCount*/,
         m_ClearValues.data()                         /*pClearValues*/
      };
      m_CommandBuffers[m_CurrentImage].beginRenderPass(renderPassBI, vk::SubpassContents::eInline);

      // Update dynamic viewport state
      vk::Viewport viewport = {
         0.0f, static_cast<float>(m_Extent.height),
         static_cast<float>(m_Extent.width), -1.0f * static_cast<float>(m_Extent.height),
         0.0f, 1.0f
      };
      m_CommandBuffers[m_CurrentImage].setViewport(0, viewport);

      // Update dynamic scissor state
      vk::Rect2D scissor = {
         {0, 0},
         m_Extent
      };
      m_CommandBuffers[m_CurrentImage].setScissor(0, scissor);
   }


   void VulkanWindowGC::EndFrame() {

      m_CommandBuffers[m_CurrentImage].endRenderPass();  // TODO: think about where render passes should begin/end

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


   void VulkanWindowGC::Bind(const VertexBuffer& buffer) {
      const VulkanVertexBuffer& vulkanVertexBuffer = reinterpret_cast<const VulkanVertexBuffer&>(buffer);
      m_CommandBuffers[m_CurrentImage].bindVertexBuffers(0, vulkanVertexBuffer.GetVkBuffer(), {0});
   }


   void VulkanWindowGC::Unbind(const VertexBuffer& buffer) {}


   void VulkanWindowGC::Bind(const IndexBuffer& buffer) {
      const VulkanIndexBuffer& vulkanIndexBuffer = reinterpret_cast<const VulkanIndexBuffer&>(buffer);
      m_CommandBuffers[m_CurrentImage].bindIndexBuffer(vulkanIndexBuffer.GetVkBuffer(), 0, vk::IndexType::eUint32);
   }


   void VulkanWindowGC::Unbind(const IndexBuffer& buffer) {}


   void VulkanWindowGC::Bind(const Texture2D& texture, uint32_t slot) {
      PKZL_NOT_IMPLEMENTED;
   }


   void VulkanWindowGC::Unbind(const Texture2D&) {
      PKZL_NOT_IMPLEMENTED;
   }

   void VulkanWindowGC::Bind(const Pipeline& pipeline) {

      // TODO: bind descriptor sets...

      const VulkanPipeline& vulkanPipeline = reinterpret_cast<const VulkanPipeline&>(pipeline);
      m_CommandBuffers[m_CurrentImage].bindPipeline(vk::PipelineBindPoint::eGraphics, vulkanPipeline.GetVkPipeline());
      m_Pipeline = &vulkanPipeline;
   }


   void VulkanWindowGC::Unbind(const Pipeline&) {
      m_Pipeline = nullptr;
   }


   std::unique_ptr<Pikzel::Pipeline> VulkanWindowGC::CreatePipeline(const PipelineSettings& settings) {
      return std::make_unique<VulkanPipeline>(m_Device, *this, settings);
   }


   void VulkanWindowGC::PushConstant(const std::string& name, bool value) {
      PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
      const ShaderPushConstant& constant = m_Pipeline->GetPushConstant(name);
      PKZL_CORE_ASSERT(constant.Type == DataType::Bool, "Push constant '{0}' type mismatch.  Bool given, expected {1}!", name, DataTypeToString(constant.Type));
      m_CommandBuffers[m_CurrentImage].pushConstants<bool>(m_Pipeline->GetVkPipelineLayout(), constant.ShaderStages, constant.Offset, value);

   }


   void VulkanWindowGC::PushConstant(const std::string& name, int value) {
      PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
      const ShaderPushConstant& constant = m_Pipeline->GetPushConstant(name);
      PKZL_CORE_ASSERT(constant.Type == DataType::Int, "Push constant '{0}' type mismatch.  Int given, expected {1}!", name, DataTypeToString(constant.Type));
      m_CommandBuffers[m_CurrentImage].pushConstants<int>(m_Pipeline->GetVkPipelineLayout(), constant.ShaderStages, constant.Offset, value);
   }


   void VulkanWindowGC::PushConstant(const std::string& name, uint32_t value) {
      PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
      const ShaderPushConstant& constant = m_Pipeline->GetPushConstant(name);
      PKZL_CORE_ASSERT(constant.Type == DataType::UInt, "Push constant '{0}' type mismatch.  UInt given, expected {1}!", name, DataTypeToString(constant.Type));
      m_CommandBuffers[m_CurrentImage].pushConstants<uint32_t>(m_Pipeline->GetVkPipelineLayout(), constant.ShaderStages, constant.Offset, value);
   }


   void VulkanWindowGC::PushConstant(const std::string& name, float value) {
      PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
      const ShaderPushConstant& constant = m_Pipeline->GetPushConstant(name);
      PKZL_CORE_ASSERT(constant.Type == DataType::Float, "Push constant '{0}' type mismatch.  Float given, expected {1}!", name, DataTypeToString(constant.Type));
      m_CommandBuffers[m_CurrentImage].pushConstants<float>(m_Pipeline->GetVkPipelineLayout(), constant.ShaderStages, constant.Offset, value);
   }


   void VulkanWindowGC::PushConstant(const std::string& name, double value) {
      PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
      const ShaderPushConstant& constant = m_Pipeline->GetPushConstant(name);
      PKZL_CORE_ASSERT(constant.Type == DataType::Double, "Push constant '{0}' type mismatch.  Double given, expected {1}!", name, DataTypeToString(constant.Type));
      m_CommandBuffers[m_CurrentImage].pushConstants<double>(m_Pipeline->GetVkPipelineLayout(), constant.ShaderStages, constant.Offset, value);
   }


   void VulkanWindowGC::PushConstant(const std::string& name, const glm::bvec2& value) {
      PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
      const ShaderPushConstant& constant = m_Pipeline->GetPushConstant(name);
      PKZL_CORE_ASSERT(constant.Type == DataType::BVec2, "Push constant '{0}' type mismatch.  BVec2 given, expected {1}!", name, DataTypeToString(constant.Type));
      m_CommandBuffers[m_CurrentImage].pushConstants<glm::bvec2>(m_Pipeline->GetVkPipelineLayout(), constant.ShaderStages, constant.Offset, value);
   }


   void VulkanWindowGC::PushConstant(const std::string& name, const glm::bvec3& value) {
      PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
      const ShaderPushConstant& constant = m_Pipeline->GetPushConstant(name);
      PKZL_CORE_ASSERT(constant.Type == DataType::BVec3, "Push constant '{0}' type mismatch.  BVec3 given, expected {1}!", name, DataTypeToString(constant.Type));
      m_CommandBuffers[m_CurrentImage].pushConstants<glm::bvec3>(m_Pipeline->GetVkPipelineLayout(), constant.ShaderStages, constant.Offset, value);
   }


   void VulkanWindowGC::PushConstant(const std::string& name, const glm::bvec4& value) {
      PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
      const ShaderPushConstant& constant = m_Pipeline->GetPushConstant(name);
      PKZL_CORE_ASSERT(constant.Type == DataType::BVec4, "Push constant '{0}' type mismatch.  BVec4 given, expected {1}!", name, DataTypeToString(constant.Type));
      m_CommandBuffers[m_CurrentImage].pushConstants<glm::bvec4>(m_Pipeline->GetVkPipelineLayout(), constant.ShaderStages, constant.Offset, value);
   }


   void VulkanWindowGC::PushConstant(const std::string& name, const glm::ivec2& value) {
      PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
      const ShaderPushConstant& constant = m_Pipeline->GetPushConstant(name);
      PKZL_CORE_ASSERT(constant.Type == DataType::IVec2, "Push constant '{0}' type mismatch.  IVec2 given, expected {1}!", name, DataTypeToString(constant.Type));
      m_CommandBuffers[m_CurrentImage].pushConstants<glm::ivec2>(m_Pipeline->GetVkPipelineLayout(), constant.ShaderStages, constant.Offset, value);
   }


   void VulkanWindowGC::PushConstant(const std::string& name, const glm::ivec3& value) {
      PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
      const ShaderPushConstant& constant = m_Pipeline->GetPushConstant(name);
      PKZL_CORE_ASSERT(constant.Type == DataType::IVec3, "Push constant '{0}' type mismatch.  IVec3 given, expected {1}!", name, DataTypeToString(constant.Type));
      m_CommandBuffers[m_CurrentImage].pushConstants<glm::ivec3>(m_Pipeline->GetVkPipelineLayout(), constant.ShaderStages, constant.Offset, value);
   }


   void VulkanWindowGC::PushConstant(const std::string& name, const glm::ivec4& value) {
      PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
      const ShaderPushConstant& constant = m_Pipeline->GetPushConstant(name);
      PKZL_CORE_ASSERT(constant.Type == DataType::IVec4, "Push constant '{0}' type mismatch.  IVec4 given, expected {1}!", name, DataTypeToString(constant.Type));
      m_CommandBuffers[m_CurrentImage].pushConstants<glm::ivec4>(m_Pipeline->GetVkPipelineLayout(), constant.ShaderStages, constant.Offset, value);
   }


   void VulkanWindowGC::PushConstant(const std::string& name, const glm::uvec2& value) {
      PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
      const ShaderPushConstant& constant = m_Pipeline->GetPushConstant(name);
      PKZL_CORE_ASSERT(constant.Type == DataType::UVec2, "Push constant '{0}' type mismatch.  UVec2 given, expected {1}!", name, DataTypeToString(constant.Type));
      m_CommandBuffers[m_CurrentImage].pushConstants<glm::uvec2>(m_Pipeline->GetVkPipelineLayout(), constant.ShaderStages, constant.Offset, value);
   }


   void VulkanWindowGC::PushConstant(const std::string& name, const glm::uvec3& value) {
      PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
      const ShaderPushConstant& constant = m_Pipeline->GetPushConstant(name);
      PKZL_CORE_ASSERT(constant.Type == DataType::UVec3, "Push constant '{0}' type mismatch.  UVec3 given, expected {1}!", name, DataTypeToString(constant.Type));
      m_CommandBuffers[m_CurrentImage].pushConstants<glm::uvec3>(m_Pipeline->GetVkPipelineLayout(), constant.ShaderStages, constant.Offset, value);
   }


   void VulkanWindowGC::PushConstant(const std::string& name, const glm::uvec4& value) {
      PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
      const ShaderPushConstant& constant = m_Pipeline->GetPushConstant(name);
      PKZL_CORE_ASSERT(constant.Type == DataType::UVec4, "Push constant '{0}' type mismatch.  UVec4 given, expected {1}!", name, DataTypeToString(constant.Type));
      m_CommandBuffers[m_CurrentImage].pushConstants<glm::uvec4>(m_Pipeline->GetVkPipelineLayout(), constant.ShaderStages, constant.Offset, value);
   }


   void VulkanWindowGC::PushConstant(const std::string& name, const glm::vec2& value) {
      PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
      const ShaderPushConstant& constant = m_Pipeline->GetPushConstant(name);
      PKZL_CORE_ASSERT(constant.Type == DataType::Vec2, "Push constant '{0}' type mismatch.  Vec2 given, expected {1}!", name, DataTypeToString(constant.Type));
      m_CommandBuffers[m_CurrentImage].pushConstants<glm::vec2>(m_Pipeline->GetVkPipelineLayout(), constant.ShaderStages, constant.Offset, value);
   }


   void VulkanWindowGC::PushConstant(const std::string& name, const glm::vec3& value) {
      PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
      const ShaderPushConstant& constant = m_Pipeline->GetPushConstant(name);
      PKZL_CORE_ASSERT(constant.Type == DataType::Vec3, "Push constant '{0}' type mismatch.  Vec3 given, expected {1}!", name, DataTypeToString(constant.Type));
      m_CommandBuffers[m_CurrentImage].pushConstants<glm::vec3>(m_Pipeline->GetVkPipelineLayout(), constant.ShaderStages, constant.Offset, value);
   }


   void VulkanWindowGC::PushConstant(const std::string& name, const glm::vec4& value) {
      PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
      const ShaderPushConstant& constant = m_Pipeline->GetPushConstant(name); // name will be like constants.mvp,  or anotherConstants.something4
      PKZL_CORE_ASSERT(constant.Type == DataType::Vec4, "Push constant '{0}' type mismatch.  Vec4 given, expected {1}!", name, DataTypeToString(constant.Type));
      m_CommandBuffers[m_CurrentImage].pushConstants<glm::vec4>(m_Pipeline->GetVkPipelineLayout(), constant.ShaderStages, constant.Offset, value);
   }


   void VulkanWindowGC::PushConstant(const std::string& name, const glm::dvec2& value) {
      PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
      const ShaderPushConstant& constant = m_Pipeline->GetPushConstant(name);
      PKZL_CORE_ASSERT(constant.Type == DataType::DVec2, "Push constant '{0}' type mismatch.  DVec2 given, expected {1}!", name, DataTypeToString(constant.Type));
      m_CommandBuffers[m_CurrentImage].pushConstants<glm::dvec2>(m_Pipeline->GetVkPipelineLayout(), constant.ShaderStages, constant.Offset, value);
   }


   void VulkanWindowGC::PushConstant(const std::string& name, const glm::dvec3& value) {
      PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
      const ShaderPushConstant& constant = m_Pipeline->GetPushConstant(name);
      PKZL_CORE_ASSERT(constant.Type == DataType::DVec3, "Push constant '{0}' type mismatch.  DVec3 given, expected {1}!", name, DataTypeToString(constant.Type));
      m_CommandBuffers[m_CurrentImage].pushConstants<glm::dvec3>(m_Pipeline->GetVkPipelineLayout(), constant.ShaderStages, constant.Offset, value);
   }


   void VulkanWindowGC::PushConstant(const std::string& name, const glm::dvec4& value) {
      PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
      const ShaderPushConstant& constant = m_Pipeline->GetPushConstant(name);
      PKZL_CORE_ASSERT(constant.Type == DataType::DVec4, "Push constant '{0}' type mismatch.  DVec4 given, expected {1}!", name, DataTypeToString(constant.Type));
      m_CommandBuffers[m_CurrentImage].pushConstants<glm::dvec4>(m_Pipeline->GetVkPipelineLayout(), constant.ShaderStages, constant.Offset, value);
   }


   void VulkanWindowGC::PushConstant(const std::string& name, const glm::mat2& value) {
      PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
      const ShaderPushConstant& constant = m_Pipeline->GetPushConstant(name);
      PKZL_CORE_ASSERT(constant.Type == DataType::Mat2, "Push constant '{0}' type mismatch.  Mat2 given, expected {1}!", name, DataTypeToString(constant.Type));
      m_CommandBuffers[m_CurrentImage].pushConstants<glm::mat2>(m_Pipeline->GetVkPipelineLayout(), constant.ShaderStages, constant.Offset, value);
   }


   void VulkanWindowGC::PushConstant(const std::string& name, const glm::mat2x3& value) {
      PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
      const ShaderPushConstant& constant = m_Pipeline->GetPushConstant(name);
      PKZL_CORE_ASSERT(constant.Type == DataType::Mat2x3, "Push constant '{0}' type mismatch.  Mat2x3 given, expected {1}!", name, DataTypeToString(constant.Type));
      m_CommandBuffers[m_CurrentImage].pushConstants<glm::mat2x3>(m_Pipeline->GetVkPipelineLayout(), constant.ShaderStages, constant.Offset, value);
   }


   void VulkanWindowGC::PushConstant(const std::string& name, const glm::mat2x4& value) {
      PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
      const ShaderPushConstant& constant = m_Pipeline->GetPushConstant(name);
      PKZL_CORE_ASSERT(constant.Type == DataType::Mat2x4, "Push constant '{0}' type mismatch.  Mat2x4 given, expected {1}!", name, DataTypeToString(constant.Type));
      m_CommandBuffers[m_CurrentImage].pushConstants<glm::mat2x4>(m_Pipeline->GetVkPipelineLayout(), constant.ShaderStages, constant.Offset, value);
   }


   void VulkanWindowGC::PushConstant(const std::string& name, const glm::mat3x2& value) {
      PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
      const ShaderPushConstant& constant = m_Pipeline->GetPushConstant(name);
      PKZL_CORE_ASSERT(constant.Type == DataType::Mat3x2, "Push constant '{0}' type mismatch.  Mat3x2 given, expected {1}!", name, DataTypeToString(constant.Type));
      m_CommandBuffers[m_CurrentImage].pushConstants<glm::mat3x2>(m_Pipeline->GetVkPipelineLayout(), constant.ShaderStages, constant.Offset, value);
   }


   void VulkanWindowGC::PushConstant(const std::string& name, const glm::mat3& value) {
      PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
      const ShaderPushConstant& constant = m_Pipeline->GetPushConstant(name);
      PKZL_CORE_ASSERT(constant.Type == DataType::Mat3, "Push constant '{0}' type mismatch.  Mat3 given, expected {1}!", name, DataTypeToString(constant.Type));
      m_CommandBuffers[m_CurrentImage].pushConstants<glm::mat3>(m_Pipeline->GetVkPipelineLayout(), constant.ShaderStages, constant.Offset, value);
   }


   void VulkanWindowGC::PushConstant(const std::string& name, const glm::mat3x4& value) {
      PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
      const ShaderPushConstant& constant = m_Pipeline->GetPushConstant(name);
      PKZL_CORE_ASSERT(constant.Type == DataType::Mat3x4, "Push constant '{0}' type mismatch.  Mat3x4 given, expected {1}!", name, DataTypeToString(constant.Type));
      m_CommandBuffers[m_CurrentImage].pushConstants<glm::mat3x4>(m_Pipeline->GetVkPipelineLayout(), constant.ShaderStages, constant.Offset, value);
   }


   void VulkanWindowGC::PushConstant(const std::string& name, const glm::mat4x2& value) {
      PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
      const ShaderPushConstant& constant = m_Pipeline->GetPushConstant(name);
      PKZL_CORE_ASSERT(constant.Type == DataType::Mat4x2, "Push constant '{0}' type mismatch.  Mat4x2 given, expected {1}!", name, DataTypeToString(constant.Type));
      m_CommandBuffers[m_CurrentImage].pushConstants<glm::mat4x2>(m_Pipeline->GetVkPipelineLayout(), constant.ShaderStages, constant.Offset, value);
   }


   void VulkanWindowGC::PushConstant(const std::string& name, const glm::mat4x3& value) {
      PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
      const ShaderPushConstant& constant = m_Pipeline->GetPushConstant(name);
      PKZL_CORE_ASSERT(constant.Type == DataType::Mat4x3, "Push constant '{0}' type mismatch.  Mat4x3 given, expected {1}!", name, DataTypeToString(constant.Type));
      m_CommandBuffers[m_CurrentImage].pushConstants<glm::mat4x3>(m_Pipeline->GetVkPipelineLayout(), constant.ShaderStages, constant.Offset, value);
   }


   void VulkanWindowGC::PushConstant(const std::string& name, const glm::mat4& value) {
      PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
      const ShaderPushConstant& constant = m_Pipeline->GetPushConstant(name); // name will be like constants.mvp,  or anotherConstants.something4
      PKZL_CORE_ASSERT(constant.Type == DataType::Mat4, "Push constant '{0}' type mismatch.  Mat4 given, expected {1}!", name, DataTypeToString(constant.Type));
      m_CommandBuffers[m_CurrentImage].pushConstants<glm::mat4>(m_Pipeline->GetVkPipelineLayout(), constant.ShaderStages, constant.Offset, value);
   }


   void VulkanWindowGC::PushConstant(const std::string& name, const glm::dmat2& value) {
      PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
      const ShaderPushConstant& constant = m_Pipeline->GetPushConstant(name);
      PKZL_CORE_ASSERT(constant.Type == DataType::DMat2, "Push constant '{0}' type mismatch.  DMat2 given, expected {1}!", name, DataTypeToString(constant.Type));
      m_CommandBuffers[m_CurrentImage].pushConstants<glm::dmat2>(m_Pipeline->GetVkPipelineLayout(), constant.ShaderStages, constant.Offset, value);
   }


   void VulkanWindowGC::PushConstant(const std::string& name, const glm::dmat2x3& value) {
      PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
      const ShaderPushConstant& constant = m_Pipeline->GetPushConstant(name);
      PKZL_CORE_ASSERT(constant.Type == DataType::DMat2x3, "Push constant '{0}' type mismatch.  DMat2x3 given, expected {1}!", name, DataTypeToString(constant.Type));
      m_CommandBuffers[m_CurrentImage].pushConstants<glm::dmat2x3>(m_Pipeline->GetVkPipelineLayout(), constant.ShaderStages, constant.Offset, value);
   }


   void VulkanWindowGC::PushConstant(const std::string& name, const glm::dmat2x4& value) {
      PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
      const ShaderPushConstant& constant = m_Pipeline->GetPushConstant(name);
      PKZL_CORE_ASSERT(constant.Type == DataType::DMat2x4, "Push constant '{0}' type mismatch.  DMat2x4 given, expected {1}!", name, DataTypeToString(constant.Type));
      m_CommandBuffers[m_CurrentImage].pushConstants<glm::dmat2x4>(m_Pipeline->GetVkPipelineLayout(), constant.ShaderStages, constant.Offset, value);
   }


   void VulkanWindowGC::PushConstant(const std::string& name, const glm::dmat3x2& value) {
      PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
      const ShaderPushConstant& constant = m_Pipeline->GetPushConstant(name);
      PKZL_CORE_ASSERT(constant.Type == DataType::DMat3x2, "Push constant '{0}' type mismatch.  DMat3x2 given, expected {1}!", name, DataTypeToString(constant.Type));
      m_CommandBuffers[m_CurrentImage].pushConstants<glm::dmat3x2>(m_Pipeline->GetVkPipelineLayout(), constant.ShaderStages, constant.Offset, value);
   }


   void VulkanWindowGC::PushConstant(const std::string& name, const glm::dmat3& value) {
      PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
      const ShaderPushConstant& constant = m_Pipeline->GetPushConstant(name);
      PKZL_CORE_ASSERT(constant.Type == DataType::DMat3, "Push constant '{0}' type mismatch.  DMat3 given, expected {1}!", name, DataTypeToString(constant.Type));
      m_CommandBuffers[m_CurrentImage].pushConstants<glm::dmat3>(m_Pipeline->GetVkPipelineLayout(), constant.ShaderStages, constant.Offset, value);
   }


   void VulkanWindowGC::PushConstant(const std::string& name, const glm::dmat3x4& value) {
      PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
      const ShaderPushConstant& constant = m_Pipeline->GetPushConstant(name);
      PKZL_CORE_ASSERT(constant.Type == DataType::DMat3x4, "Push constant '{0}' type mismatch.  DMat3x4 given, expected {1}!", name, DataTypeToString(constant.Type));
      m_CommandBuffers[m_CurrentImage].pushConstants<glm::dmat3x4>(m_Pipeline->GetVkPipelineLayout(), constant.ShaderStages, constant.Offset, value);
   }


   void VulkanWindowGC::PushConstant(const std::string& name, const glm::dmat4x2& value) {
      PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
      const ShaderPushConstant& constant = m_Pipeline->GetPushConstant(name);
      PKZL_CORE_ASSERT(constant.Type == DataType::DMat4x2, "Push constant '{0}' type mismatch.  DMat4x2 given, expected {1}!", name, DataTypeToString(constant.Type));
      m_CommandBuffers[m_CurrentImage].pushConstants<glm::dmat4x2>(m_Pipeline->GetVkPipelineLayout(), constant.ShaderStages, constant.Offset, value);
   }


   void VulkanWindowGC::PushConstant(const std::string& name, const glm::dmat4x3& value) {
      PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
      const ShaderPushConstant& constant = m_Pipeline->GetPushConstant(name);
      PKZL_CORE_ASSERT(constant.Type == DataType::DMat4x3, "Push constant '{0}' type mismatch.  DMat4x3 given, expected {1}!", name, DataTypeToString(constant.Type));
      m_CommandBuffers[m_CurrentImage].pushConstants<glm::dmat4x3>(m_Pipeline->GetVkPipelineLayout(), constant.ShaderStages, constant.Offset, value);
   }


   void VulkanWindowGC::PushConstant(const std::string& name, const glm::dmat4& value) {
      PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
      const ShaderPushConstant& constant = m_Pipeline->GetPushConstant(name);
      PKZL_CORE_ASSERT(constant.Type == DataType::DMat4, "Push constant '{0}' type mismatch.  DMat4 given, expected {1}!", name, DataTypeToString(constant.Type));
      m_CommandBuffers[m_CurrentImage].pushConstants<glm::dmat4>(m_Pipeline->GetVkPipelineLayout(), constant.ShaderStages, constant.Offset, value);
   }


   void VulkanWindowGC::DrawIndexed(const VertexBuffer& vertexBuffer, const IndexBuffer& indexBuffer, uint32_t indexCount /*= 0*/) {
      GCBinder bindVB {*this, vertexBuffer};
      GCBinder bindIB {*this, indexBuffer};
      m_CommandBuffers[m_CurrentImage].drawIndexed(indexBuffer.GetCount(), 1, 0, 0, 0);
   }


   void VulkanWindowGC::CreateSurface() {
      VkSurfaceKHR surface;
      if (glfwCreateWindowSurface(m_Device->GetVkInstance(), m_Window, nullptr, &surface) != VK_SUCCESS) {
         throw std::runtime_error("failed to create window surface!");
      }
      if (!m_Device->GetVkPhysicalDevice().getSurfaceSupportKHR(m_Device->GetPresentQueueFamilyIndex(), surface)) {
         throw std::runtime_error("Vulkan physical device does not support window surface!");
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
