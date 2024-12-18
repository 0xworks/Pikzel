#include "OpenGLFramebuffer.h"
#include "OpenGLGraphicsContext.h"

#include "Pikzel/Renderer/RenderCore.h"

#include <GL/gl.h>

namespace Pikzel {

   static GLenum buffers[4] = {
      GL_COLOR_ATTACHMENT0,
      GL_COLOR_ATTACHMENT1,
      GL_COLOR_ATTACHMENT2,
      GL_COLOR_ATTACHMENT3,
   };

   GLenum OpenGLAttachmentType(const AttachmentType type, const int index) {
      switch (type) {
         case AttachmentType::Color: {
            switch (index) {
               case 0:  return GL_COLOR_ATTACHMENT0;
               case 1:  return GL_COLOR_ATTACHMENT1;
               case 2:  return GL_COLOR_ATTACHMENT2;
               case 3:  return GL_COLOR_ATTACHMENT3;
               default:
                  PKZL_CORE_ASSERT(false, "Framebuffer color attachment index out of range! Must be 0 <= index <= 3");
            }
         }
         case AttachmentType::Depth:        return GL_DEPTH_ATTACHMENT;
         case AttachmentType::DepthStencil: return GL_DEPTH_STENCIL_ATTACHMENT;
         //case AttachmentType::Stencil:      return GL_STENCIL_ATTACHMENT;
      }
      PKZL_CORE_ASSERT(false, "Unsupported AttachmentType!");
      return 0;
   }


   OpenGLFramebuffer::OpenGLFramebuffer(const FramebufferSettings& settings)
   : m_Settings(settings)
   {
      glGenFramebuffers(1, &m_RendererId);
      if (settings.msaaNumSamples > 1) {
         glGenFramebuffers(1, &m_ResolveRendererId);
      }
      CreateAttachments();
      m_Context = std::make_unique<OpenGLFramebufferGC>(this);
   }


   OpenGLFramebuffer::~OpenGLFramebuffer() {
      glBindFramebuffer(GL_FRAMEBUFFER, 0);
      glDeleteFramebuffers(1, &m_ResolveRendererId);
      glDeleteFramebuffers(1, &m_RendererId);
   }


   GraphicsContext& OpenGLFramebuffer::GetGraphicsContext() {
      PKZL_CORE_ASSERT(m_Context, "Accessing null graphics context!");
      return *m_Context;
   }


   uint32_t OpenGLFramebuffer::GetWidth() const {
      return m_Settings.width;
   }


   uint32_t OpenGLFramebuffer::GetHeight() const {
      return m_Settings.height;
   }


   void OpenGLFramebuffer::Resize(const uint32_t width, const uint32_t height) {
      if ((m_Settings.width != width) || (m_Settings.height != height)) {
         glBindFramebuffer(GL_FRAMEBUFFER, 0);  // The idea here is to make sure the GPU isnt still rendering to the attachments we are about to destroy
                                                // but is just binding to the default framebuffer sufficient to ensure that?
         if (m_MSAADepthStencilAttachmentRendererId) {
            glDeleteTextures(1, &m_MSAADepthStencilAttachmentRendererId);
            m_MSAADepthStencilAttachmentRendererId = 0;
         }
         m_DepthTexture.reset();

         glDeleteTextures(m_MSAAColorAttachmentRendererIds.size(), m_MSAAColorAttachmentRendererIds.data());
         m_MSAAColorAttachmentRendererIds.clear();
         m_ColorTextures.clear();

         m_Settings.width = width;
         m_Settings.height = height;

         CreateAttachments();
      }
   }


   uint32_t OpenGLFramebuffer::GetMSAANumSamples() const {
      return m_Settings.msaaNumSamples;
   }


   const glm::vec4& OpenGLFramebuffer::GetClearColorValue() const {
      return m_Settings.clearColorValue;
   }


   double OpenGLFramebuffer::GetClearDepthValue() const {
      return m_Settings.clearDepthValue;
   }

   uint32_t OpenGLFramebuffer::GetNumColorAttachments() const {
      return m_ColorTextures.size();
   }


   const Texture& OpenGLFramebuffer::GetColorTexture(const int index) const {
      return *m_ColorTextures[index];
   }


   bool OpenGLFramebuffer::HasDepthAttachment() const {
      return m_DepthTexture != nullptr;
   }


   const Texture& OpenGLFramebuffer::GetDepthTexture() const {
      PKZL_CORE_ASSERT(m_DepthTexture, "Accessing null depth texture!  Did you create the frame buffer with a depth texture attachment?");
      return *m_DepthTexture;
   }


   ImTextureID OpenGLFramebuffer::GetImGuiColorTextureId(const int index) const {
      Texture& texture = *m_ColorTextures[index];
      return (ImTextureID)(intptr_t)static_cast<OpenGLTexture&>(texture).GetRendererId();
   }


   ImTextureID OpenGLFramebuffer::GetImGuiDepthTextureId() const {
      PKZL_CORE_ASSERT(m_DepthTexture, "Accessing null depth texture!  Did you create the frame buffer with a depth texture attachment?");
      return (ImTextureID)(intptr_t)static_cast<OpenGLTexture&>(*m_DepthTexture).GetRendererId();
   }


   uint32_t OpenGLFramebuffer::GetRendererId() const {
      return m_RendererId;
   }


   uint32_t OpenGLFramebuffer::GetResolveRendererId() const {
      return m_ResolveRendererId;
   }

   void OpenGLFramebuffer::CreateAttachments() {
      glBindFramebuffer(GL_FRAMEBUFFER, m_RendererId);

      bool isMultiSampled = m_Settings.msaaNumSamples > 1;

      int numColorAttachments = 0;
      int numDepthAttachments = 0;
      GLenum depthAttachmentType = GL_DEPTH_STENCIL_ATTACHMENT;
      for (const auto attachment : m_Settings.attachments) {
         switch(attachment.attachmentType) {
            case AttachmentType::Color: {
               PKZL_CORE_ASSERT(numColorAttachments < 4, "Framebuffer can have a maximum of four color attachments!");
               if (isMultiSampled) {
                  PKZL_CORE_ASSERT(attachment.textureType == TextureType::Texture2D, "Framebuffer MSAA not supported for cube map texture color attachment");
                  m_MSAAColorAttachmentRendererIds.emplace_back(0);
                  glGenTextures(1, &m_MSAAColorAttachmentRendererIds.back());
                  glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, m_MSAAColorAttachmentRendererIds.back());
                  glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, m_Settings.msaaNumSamples, TextureFormatToInternalFormat(attachment.format), m_Settings.width, m_Settings.height, GL_TRUE);
                  glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
                  glFramebufferTexture2D(GL_FRAMEBUFFER, OpenGLAttachmentType(attachment.attachmentType, numColorAttachments), GL_TEXTURE_2D_MULTISAMPLE, m_MSAAColorAttachmentRendererIds.back(), 0);
               }
               m_ColorTextures.emplace_back(RenderCore::CreateTexture({
                  .textureType = attachment.textureType,
                  .width = m_Settings.width,
                  .height = m_Settings.height,
                  .layers = m_Settings.layers,
                  .format = attachment.format,
                  .wrapU = TextureWrap::ClampToEdge,
                  .wrapV = TextureWrap::ClampToEdge,
                  .mipLevels = 1
               }));
               ++numColorAttachments;
               break;
            }
            case AttachmentType::Depth:
            case AttachmentType::DepthStencil: {
               PKZL_CORE_ASSERT(numDepthAttachments == 0, "Framebuffer can have a maximum of one depth attachment!");
               depthAttachmentType = OpenGLAttachmentType(attachment.attachmentType, numDepthAttachments);
               if (isMultiSampled) {
                  PKZL_CORE_ASSERT(attachment.textureType == TextureType::Texture2D, "Framebuffer MSAA not supported for cube map texture depth attachment");
                  glGenTextures(1, &m_MSAADepthStencilAttachmentRendererId);
                  glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, m_MSAADepthStencilAttachmentRendererId);
                  glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, m_Settings.msaaNumSamples, TextureFormatToInternalFormat(attachment.format), m_Settings.width, m_Settings.height, GL_TRUE);
                  glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
                  glFramebufferTexture2D(GL_FRAMEBUFFER, depthAttachmentType, GL_TEXTURE_2D_MULTISAMPLE, m_MSAADepthStencilAttachmentRendererId, 0);
               }
               m_DepthTexture = RenderCore::CreateTexture({
                  .textureType = attachment.textureType,
                  .width = m_Settings.width,
                  .height = m_Settings.height,
                  .layers = m_Settings.layers,
                  .format = attachment.format,
                  .wrapU = TextureWrap::ClampToBorder,
                  .wrapV = TextureWrap::ClampToBorder,
                  .mipLevels = 1
               });
               ++numDepthAttachments;
               break;
            }
            default: {
               PKZL_CORE_ASSERT(false, "Unknown attachment type!");
            }
         }
      }

      // resolve FBO
      if (m_ResolveRendererId) {
         glDrawBuffers(numColorAttachments, buffers);
         if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            PKZL_CORE_LOG_FATAL("Framebuffer is not complete!");
         }
         glBindFramebuffer(GL_FRAMEBUFFER, m_ResolveRendererId);
      }

      int i = 0;
      for(const auto& colorTexture : m_ColorTextures) {
         glFramebufferTexture(GL_FRAMEBUFFER, OpenGLAttachmentType(AttachmentType::Color, i), static_cast<OpenGLTexture&>(*colorTexture).GetRendererId(), 0);
         ++i;
      }
      if (m_DepthTexture) {
         glFramebufferTexture(GL_FRAMEBUFFER, depthAttachmentType, static_cast<OpenGLTexture&>(*m_DepthTexture).GetRendererId(), 0);
      }
      glDrawBuffers(numColorAttachments, buffers);
      if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
         PKZL_CORE_LOG_FATAL("Framebuffer is not complete!");
      }
      glBindFramebuffer(GL_FRAMEBUFFER, 0);
   }
}
