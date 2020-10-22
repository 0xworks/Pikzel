#pragma once

#include "Core.h"
#include "Pikzel/Renderer/GraphicsContext.h"

#include <glm/glm.hpp>
#include <memory>

namespace Pikzel {

   class Window {
   public:

      struct Settings {
         const char* Title = "Pikzel Engine";
         uint32_t Width = 1280;
         uint32_t Height = 720;
         glm::vec4 ClearColor = {0.0f, 0.0f, 0.0f, 1.0f};
         bool IsResizable = true;
         bool IsFullScreen = false;
         bool IsCursorEnabled = true;
         bool IsVSync = true;
         uint32_t MinWidth = 0;
         uint32_t MinHeight = 0;
         uint32_t MaxWidth = 0;
         uint32_t MaxHeight = 0;
      };

      virtual ~Window() = default;

      virtual void* GetNativeWindow() const = 0;

      virtual uint32_t GetWidth() const = 0;
      virtual uint32_t GetHeight() const = 0;

      virtual glm::vec4 GetClearColor() const = 0;

      virtual void SetVSync(bool enabled) = 0;
      virtual bool IsVSync() const = 0;

      virtual float ContentScale() const = 0;


      virtual void BeginFrame() = 0;
      virtual void EndFrame() = 0;

      // These shouldn't be part of Window. (e.g. maybe client wants to use Nano GUI instead, and doesn't want any ImGui baggage)
      // TODO: move these somewhere else
      virtual void InitializeImGui() = 0;
      virtual void UploadImGuiFonts() = 0;
      virtual void BeginImGuiFrame() = 0;
      virtual void EndImGuiFrame() = 0;

      virtual GraphicsContext& GetGraphicsContext() = 0;

      virtual glm::vec2 GetCursorPos() const = 0;


   public:
      static std::unique_ptr<Window> Create(const Settings& settings = {});
   };

}
