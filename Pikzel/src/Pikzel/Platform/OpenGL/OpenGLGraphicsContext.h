#pragma once

#include "Pikzel/Renderer/GraphicsContext.h"

struct GLFWwindow;

namespace Pikzel {

   class OpenGLGraphicsContext : public GraphicsContext {
   public:
      OpenGLGraphicsContext(GLFWwindow* window);  // note: ownership is not transferred, and destructor for OpenGLContext does not destroy the window.
      virtual void SwapBuffers() override;

   private:
      GLFWwindow* m_Window;  // OpenGLContext does not own the window!

   private:
      static bool s_OpenGLInitialized;
   };

}
