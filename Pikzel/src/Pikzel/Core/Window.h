#pragma once

#include "Core.h"

#include <memory>

namespace Pikzel {

   struct WindowSettings {
      const char* Title = "Pikzel Engine";
      uint32_t Width = 1280;
      uint32_t Height = 720;
      bool IsResizable = true;
      bool IsFullScreen = false;
      bool IsCursorEnabled = true;
   };


   class GraphicsContext;

   class Window {
   public:
      virtual ~Window() = default;

      virtual void* GetNativeWindow() const = 0;

      virtual uint32_t GetWidth() const = 0;
      virtual uint32_t GetHeight() const = 0;

      virtual void SetVSync(bool enabled) = 0;
      virtual bool IsVSync() const = 0;

      virtual GraphicsContext& GetGraphicsContext() = 0;

      virtual void Update() = 0;

      virtual float ContentScale() const = 0;

   public:
      static std::unique_ptr<Window> Create(const WindowSettings& settings = WindowSettings());
   };

}
