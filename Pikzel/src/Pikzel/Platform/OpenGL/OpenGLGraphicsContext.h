#pragma once

#include "Pikzel/Core/Window.h"
#include "Pikzel/Renderer/GraphicsContext.h"

struct GLFWwindow;

namespace Pikzel {

   class OpenGLGraphicsContext : public GraphicsContext {
   public:
      OpenGLGraphicsContext(const Window& window);

      virtual void BeginFrame() override;
      virtual void EndFrame() override;

      virtual void SwapBuffers() override;

   private:
      GLFWwindow* m_WindowHandle;
   };

}