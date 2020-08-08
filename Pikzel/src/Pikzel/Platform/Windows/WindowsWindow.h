#pragma once

#include "Pikzel/Core/Window.h"

#include <memory>
#include <string>

struct GLFWwindow;

namespace Pikzel {

   class GraphicsContext;

   class WindowsWindow : public Window {
   public:
      WindowsWindow(const WindowSettings& settings);
      virtual ~WindowsWindow();

      virtual void* GetNativeWindow() const override;

      virtual uint32_t GetWidth() const override;
      virtual uint32_t GetHeight() const override;

      virtual void SetVSync(bool enabled) override;
      virtual bool IsVSync() const override;

      virtual void Update() override;

   private:
      WindowSettings m_Settings;

      std::unique_ptr<GraphicsContext> m_Context;
      GLFWwindow* m_Window;

      bool m_VSync;
   };

}
