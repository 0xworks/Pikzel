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
      virtual const glm::vec4& GetClearColorValue() const override;
      virtual double GetClearDepthValue() const override;

      virtual uint32_t GetNumColorAttachments() const override;
      virtual const Texture& GetColorTexture(const int index) const override;

      virtual bool HasDepthAttachment() const override;
      virtual const Texture& GetDepthTexture() const override;

      virtual ImTextureID GetImGuiColorTextureId(const int index) const override;
      virtual ImTextureID GetImGuiDepthTextureId() const override;

   public:
      vk::Framebuffer GetVkFramebuffer() const;
      std::vector<vk::AttachmentDescription2>& GetVkAttachments();

      void TransitionDepthImageLayout(const vk::ImageLayout oldLayout, const vk::ImageLayout newLayout);

   private:
      void CreateAttachments();
      void DestroyAttachments();

      void CreateFramebuffer();
      void DestroyFramebuffer();

   private:
      FramebufferSettings m_Settings;
      std::shared_ptr<VulkanDevice> m_Device;

      std::vector<std::unique_ptr<Texture>> m_ColorTextures;
      std::vector < std::unique_ptr<VulkanImage>> m_MSAAColorImages;
      std::unique_ptr<Texture> m_DepthTexture;
      std::unique_ptr<VulkanImage> m_MSAADepthImage;
      std::unique_ptr<GraphicsContext> m_Context;

      std::vector<vk::ImageView> m_ImageViews;
      std::vector<vk::AttachmentDescription2> m_Attachments;


      vk::Framebuffer m_Framebuffer;
      mutable std::vector<VkDescriptorSet> m_ColorDescriptorSets;
      mutable VkDescriptorSet m_DepthDescriptorSet = VK_NULL_HANDLE;
      uint32_t m_LayerCount = 0;
   };
}
