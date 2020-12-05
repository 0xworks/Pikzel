#pragma once

#include "OpenGLTexture.h"
#include "Pikzel/Renderer/Framebuffer.h"

namespace Pikzel {

   class OpenGLFramebuffer : public Framebuffer {
   public:
      OpenGLFramebuffer(const FramebufferSettings& settings);
      ~OpenGLFramebuffer();

      virtual GraphicsContext& GetGraphicsContext() override;

      virtual uint32_t GetWidth() const override;
      virtual uint32_t GetHeight() const override;
      virtual void Resize(const uint32_t width, const uint32_t height) override;

      virtual uint32_t GetMSAANumSamples() const override;

      virtual const glm::vec4& GetClearColor() const override;

      virtual const Texture2D& GetColorTexture(const int index) const override;
      virtual const Texture2D& GetDepthTexture() const override;

      virtual ImTextureID GetImGuiColorTextureId(const int index) const override;
      virtual ImTextureID GetImGuiDepthTextureId() const override;

   public:
      uint32_t GetRendererId() const;
      uint32_t GetResolveRendererId() const;

   private:
      void CreateAttachments();
   
   private:
      FramebufferSettings m_Settings;
      std::vector<std::unique_ptr<OpenGLTexture2D>> m_ColorTextures;
      std::unique_ptr<OpenGLTexture2D> m_DepthTexture;
      std::unique_ptr<GraphicsContext> m_Context;
      uint32_t m_RendererId = {};
      uint32_t m_ResolveRendererId = {};
      std::vector<uint32_t> m_MSAAColorAttachmentRendererIds;
      uint32_t m_MSAADepthStencilAttachmentRendererId = {};
   };

}
