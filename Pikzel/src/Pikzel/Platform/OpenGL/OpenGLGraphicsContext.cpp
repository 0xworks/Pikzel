#include "glpch.h"
#include "OpenGLGraphicsContext.h"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

namespace Pikzel {

   OpenGLGraphicsContext::OpenGLGraphicsContext(Window& window)
   : m_WindowHandle((GLFWwindow*)window.GetNativeWindow())
   {
      PKZL_CORE_ASSERT(m_WindowHandle, "Window handle is null!")
   }


   void OpenGLGraphicsContext::BeginFrame() {
      glfwMakeContextCurrent(m_WindowHandle);
   }


   void OpenGLGraphicsContext::EndFrame() {
      ;
   }


   void OpenGLGraphicsContext::SwapBuffers() {
      PKZL_PROFILE_FUNCTION();
      glfwSwapBuffers(m_WindowHandle);
   }

}
