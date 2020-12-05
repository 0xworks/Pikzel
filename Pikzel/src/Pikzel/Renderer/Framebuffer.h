#pragma once

#include "GraphicsContext.h"
#include <imgui.h>

namespace Pikzel {

   enum class AttachmentType {
      Color,
      Depth,
      DepthStencil,
      // Stencil   // TODO
   };


   struct PKZL_API FramebufferAttachmentSettings {
      AttachmentType Type = AttachmentType::Color;
      TextureFormat Format = TextureFormat::SRGB8;
   };


   struct PKZL_API FramebufferSettings {
      uint32_t Width = 1920;
      uint32_t Height = 1080;
      uint32_t MSAANumSamples = 1;
      glm::vec4 ClearColor = {};
      std::vector<FramebufferAttachmentSettings> Attachments = {{AttachmentType::Color, TextureFormat::SRGB8}, {AttachmentType::Depth, TextureFormat::D32F}};
   };


   class PKZL_API Framebuffer {
   public:
      virtual ~Framebuffer() = default;

      virtual GraphicsContext& GetGraphicsContext() = 0;

      virtual uint32_t GetWidth() const = 0;
      virtual uint32_t GetHeight() const = 0;
      virtual void Resize(const uint32_t width, const uint32_t height) = 0;

      virtual uint32_t GetMSAANumSamples() const = 0;

      virtual const glm::vec4& GetClearColor() const = 0;

      virtual const Texture2D& GetColorTexture(const int index) const = 0;
      virtual const Texture2D& GetDepthTexture() const = 0;

      // for using the contents of the framebuffer in a call to ImGui::Image()
      virtual ImTextureID GetImGuiColorTextureId(const int index) const = 0;
      virtual ImTextureID GetImGuiDepthTextureId() const = 0;
   };

}
