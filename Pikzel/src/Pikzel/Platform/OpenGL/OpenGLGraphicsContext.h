#pragma once

#include "Pikzel/Events/WindowEvents.h"
#include "Pikzel/Renderer/GraphicsContext.h"

struct GLFWwindow;


namespace Pikzel {

   class OpenGLGraphicsContext : public GraphicsContext {
   public:
      OpenGLGraphicsContext(GLFWwindow* window);  // note: ownership is not transferred, and destructor for OpenGLContext does not destroy the window.
      virtual ~OpenGLGraphicsContext();

      virtual void UploadImGuiFonts() override;

      virtual void BeginFrame() override;
      virtual void EndFrame() override;

      virtual void BeginImGuiFrame() override;
      virtual void EndImGuiFrame() override;

      virtual void SwapBuffers() override;

   private:
      void OnWindowResize(const WindowResizeEvent& event);

   private:
      GLFWwindow* m_Window;  // OpenGLContext does not own the window!
      bool m_ImGuiFrameStarted;

   private:
      static bool s_OpenGLInitialized;
   };

}
