#pragma once

#include "OpenGLTexture.h"
#include "Pikzel/Renderer/Framebuffer.h"

namespace Pikzel {

   class OpenGLFramebuffer : public Framebuffer {
   public:
      OpenGLFramebuffer(const FramebufferSettings& settings);
      ~OpenGLFramebuffer() = default;

      virtual GraphicsContext& GetGraphicsContext() override;
      virtual const glm::vec4& GetClearColor() const override;

      virtual uint32_t GetWidth() const override;
      virtual uint32_t GetHeight() const override;

      virtual void Resize(const uint32_t width, const uint32_t height) override;

      virtual const Texture2D& GetColorTexture() const override;
      virtual ImTextureID GetImGuiTextureId() override;

   public:
      uint32_t GetRendererId() const;

   private:
      FramebufferSettings m_Settings;
      std::unique_ptr<OpenGLTexture2D> m_Texture;
      std::unique_ptr<GraphicsContext> m_Context;
      uint32_t m_RendererId;
      uint32_t m_DepthStencilAttachmentRendererId;

   };

}
