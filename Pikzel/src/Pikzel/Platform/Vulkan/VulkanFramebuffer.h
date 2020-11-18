#pragma once

#include "VulkanDevice.h"
#include "VulkanTexture.h"
#include "Pikzel/Renderer/Framebuffer.h"

#include <memory>

namespace Pikzel {

   class VulkanFramebuffer : public Framebuffer {
   public:
      VulkanFramebuffer(std::shared_ptr<VulkanDevice> device, const FramebufferSettings& settings);
      ~VulkanFramebuffer();

      virtual GraphicsContext& GetGraphicsContext() override;

      virtual uint32_t GetWidth() const override;
      virtual uint32_t GetHeight() const override;
      virtual void Resize(const uint32_t width, const uint32_t height) override;

      virtual uint32_t GetMSAANumSamples() const override;
      virtual const glm::vec4& GetClearColor() const override;

      virtual const Texture2D& GetColorTexture() const override;
      virtual ImTextureID GetImGuiTextureId() override;

   public:
      vk::Framebuffer GetVkFramebuffer() const;
      vk::Format GetVkFormat() const;
      vk::Format GetVkDepthFormat() const;

   private:
      FramebufferSettings m_Settings;
      std::shared_ptr<VulkanDevice> m_Device;

      std::unique_ptr<VulkanTexture2D> m_Texture;
      std::unique_ptr<VulkanImage> m_ColorImage;
      std::unique_ptr<VulkanImage> m_DepthImage;
      std::unique_ptr<GraphicsContext> m_Context;

      vk::Framebuffer m_Framebuffer;
      VkDescriptorSet m_DescriptorSet = VK_NULL_HANDLE;
      vk::Format m_DepthFormat = vk::Format::eUndefined;
   };
}
