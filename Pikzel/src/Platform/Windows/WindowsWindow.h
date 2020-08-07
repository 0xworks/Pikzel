#pragma once

#include "Pikzel/Core/Window.h"

#include <memory>
#include <string>

struct GLFWwindow;

namespace Pikzel {

   //class GraphicsContext;

   class WindowsWindow : public Window {
   public:
      WindowsWindow(const WindowSettings& settings);
      virtual ~WindowsWindow();

      void OnUpdate() override;

      uint32_t GetWidth() const override;
      uint32_t GetHeight() const override;

      void SetVSync(bool enabled) override;
      bool IsVSync() const override;

      virtual void* GetNativeWindow() const override;

   private:
      WindowSettings m_Settings;

      //std::unique_ptr<GraphicsContext> m_Context;
      GLFWwindow* m_Window;

      bool m_VSync;
   };

}
