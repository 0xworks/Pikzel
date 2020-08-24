#pragma once

#include "Pikzel/Events/WindowEvents.h"
#include "Pikzel/Renderer/GraphicsContext.h"
#include "DescriptorBinding.h"
#include "QueueFamilyIndices.h"
#include "VulkanDevice.h"
#include "VulkanImage.h"

struct GLFWwindow;

namespace Pikzel {

   class VulkanGraphicsContext : public GraphicsContext {
   public:
      VulkanGraphicsContext(std::shared_ptr<VulkanDevice> device);
      virtual ~VulkanGraphicsContext();

      virtual void UploadImGuiFonts() override;
      virtual void BeginImGuiFrame() override;
      virtual void EndImGuiFrame() override;

   protected:

      void CreateDepthStencil();
      void DestroyDepthStencil();

      vk::RenderPass CreateRenderPass();
      void DestroyRenderPass(vk::RenderPass renderPass);

      vk::DescriptorPool CreateDescriptorPool(const vk::ArrayProxy<DescriptorBinding>& descriptorBindings, size_t maxSets);
      void DestroyDescriptorPool(vk::DescriptorPool descriptorPool);

      void CreateCommandPool();
      void DestroyCommandPool();

      void CreateCommandBuffers(const uint32_t commandBufferCount);
      void DestroyCommandBuffers();

      void CreatePipelineCache();
      void DestroyPipelineCache();

      void SubmitSingleTimeCommands(const std::function<void(vk::CommandBuffer)>& action);

   protected:
      std::shared_ptr<VulkanDevice> m_Device;

      vk::Format m_Format = vk::Format::eUndefined;
      vk::Extent2D m_Extent;

      vk::Format m_DepthFormat;
      std::unique_ptr<VulkanImage> m_DepthImage;

      vk::RenderPass m_RenderPass;

      vk::CommandPool m_CommandPool;
      std::vector<vk::CommandBuffer> m_CommandBuffers;

      vk::PipelineCache m_PipelineCache;

   };

}
