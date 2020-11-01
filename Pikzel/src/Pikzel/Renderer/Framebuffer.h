#pragma once

#include "GraphicsContext.h"
#include <imgui.h>

namespace Pikzel {

   struct FramebufferSettings {
      uint32_t Width = 1920;
      uint32_t Height = 1080;
      glm::vec4 ClearColor = {};
   };


   class Framebuffer {
   public:
      virtual ~Framebuffer() = default;

      virtual GraphicsContext& GetGraphicsContext() = 0;

      virtual const glm::vec4& GetClearColor() const = 0;

      virtual uint32_t GetWidth() const = 0;
      virtual uint32_t GetHeight() const = 0;
      virtual void Resize(const uint32_t width, const uint32_t height) = 0;

      virtual const Texture2D& GetColorTexture() const = 0;

      // for using the contents of the framebuffer in a call to ImGui::Image()
      virtual ImTextureID GetImGuiTextureId() = 0;
   };

}
