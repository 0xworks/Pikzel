#pragma once

#include "Pikzel/Core/Window.h"

#include <memory>

struct GLFWwindow;

namespace Pikzel {

   class PKZL_API GLFWWindow : public Window {
   public:
      GLFWWindow(const Settings& settings);
      virtual ~GLFWWindow();

      virtual void* GetNativeWindow() const override;

      virtual uint32_t GetWidth() const override;
      virtual uint32_t GetHeight() const override;

      virtual uint32_t GetMSAANumSamples() const override;

      virtual glm::vec4 GetClearColor() const override;

      virtual void SetVSync(bool enabled) override;
      virtual bool IsVSync() const override;

      virtual float ContentScale() const override;

      virtual void BeginFrame() override;
      virtual void EndFrame() override;

      virtual void InitializeImGui() override;
      virtual void BeginImGuiFrame() override;
      virtual void EndImGuiFrame() override;

      virtual GraphicsContext& GetGraphicsContext() override;

      virtual glm::vec2 GetCursorPos() const override;

   private:
      Settings m_Settings;

      std::unique_ptr<GraphicsContext> m_Context;
      GLFWwindow* m_Window;
   };

}
