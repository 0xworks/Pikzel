#include "OpenGLFramebuffer.h"
#include "OpenGLGraphicsContext.h"

namespace Pikzel {

   OpenGLFramebuffer::OpenGLFramebuffer(const FramebufferSettings& settings)
   : m_Settings(settings)
   {
      glGenFramebuffers(1, &m_RendererId);
      glBindFramebuffer(GL_FRAMEBUFFER, m_RendererId);

      m_Texture = std::make_unique<OpenGLTexture2D>(settings.Width, settings.Height, TextureFormat::RGBA8, 1);
      glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_Texture->GetRendererId(), 0);

      glGenRenderbuffers(1, &m_DepthStencilAttachmentRendererId);
      glBindRenderbuffer(GL_RENDERBUFFER, m_DepthStencilAttachmentRendererId);
      glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, settings.Width, settings.Height);
      glBindRenderbuffer(GL_RENDERBUFFER, 0);
      glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_DepthStencilAttachmentRendererId);

      if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
         PKZL_CORE_LOG_FATAL("Framebuffer is not complete!");
      }
      glBindFramebuffer(GL_FRAMEBUFFER, 0);

      m_Context = std::make_unique<OpenGLFramebufferGC>(this);
   }


   GraphicsContext& OpenGLFramebuffer::GetGraphicsContext() {
      PKZL_CORE_ASSERT(m_Context, "Accessing null graphics context!");
      return *m_Context;
   }


   const glm::vec4& OpenGLFramebuffer::GetClearColor() const {
      return m_Settings.ClearColor;
   }


   uint32_t OpenGLFramebuffer::GetWidth() const {
      return m_Settings.Width;
   }


   uint32_t OpenGLFramebuffer::GetHeight() const {
      return m_Settings.Height;
   }


   void OpenGLFramebuffer::Resize(const uint32_t width, const uint32_t height) {
      glBindFramebuffer(GL_FRAMEBUFFER, 0);  // The idea here is to make sure the GPU isnt still rendering to the attachments we are about to destroy
                                             // but is just binding to the default framebuffer sufficient to ensure that?
      m_Texture.reset();
      glDeleteRenderbuffers(1, &m_DepthStencilAttachmentRendererId);
      m_DepthStencilAttachmentRendererId = 0;

      glBindFramebuffer(GL_FRAMEBUFFER, m_RendererId);

      m_Settings.Width = width;
      m_Settings.Height = height;

      m_Texture = std::make_unique<OpenGLTexture2D>(m_Settings.Width, m_Settings.Height, TextureFormat::RGBA8, 1);
      glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_Texture->GetRendererId(), 0);

      glGenRenderbuffers(1, &m_DepthStencilAttachmentRendererId);
      glBindRenderbuffer(GL_RENDERBUFFER, m_DepthStencilAttachmentRendererId);
      glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, m_Settings.Width, m_Settings.Height);
      glBindRenderbuffer(GL_RENDERBUFFER, 0);
      glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_DepthStencilAttachmentRendererId);

      if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
         PKZL_CORE_LOG_FATAL("Framebuffer is not complete!");
      }
      glBindFramebuffer(GL_FRAMEBUFFER, 0);
   }


   const Texture2D& OpenGLFramebuffer::GetColorTexture() const {
      return *m_Texture;
   }


   ImTextureID OpenGLFramebuffer::GetImGuiTextureId() {
      return (ImTextureID)(intptr_t)m_Texture->GetRendererId();
   }

   uint32_t OpenGLFramebuffer::GetRendererId() const {
      return m_RendererId;
   }

}