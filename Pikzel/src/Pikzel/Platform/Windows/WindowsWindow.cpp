#include "pch.h"
#include "WindowsWindow.h"

#include "Pikzel/Events/EventDispatcher.h"
#include "Pikzel/Events/KeyEvents.h"
#include "Pikzel/Events/MouseEvents.h"
#include "Pikzel/Events/WindowEvents.h"
#include "Pikzel/Renderer/RenderCore.h"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

namespace Pikzel {

   std::unique_ptr<Window> Window::Create(const WindowSettings& settings) {
      return std::make_unique<WindowsWindow>(settings);
   }


   static uint8_t s_GLFWWindowCount = 0;


   WindowsWindow::WindowsWindow(const WindowSettings& settings) {
      m_Settings = settings;

      PKZL_CORE_LOG_INFO("Platform Windows:");
      PKZL_CORE_LOG_INFO("  Title: {0}", m_Settings.Title);
      PKZL_CORE_LOG_INFO("  Size: ({0}, {1})", m_Settings.Width, m_Settings.Height);

      if (s_GLFWWindowCount == 0) {
         PKZL_PROFILE_SCOPE("glfwInit");
         if (!glfwInit()) {
            throw std::runtime_error("Could not initialize GLFW!");
         }
         glfwSetErrorCallback([] (int error, const char* description) {
            PKZL_CORE_LOG_ERROR("GLFW Error ({0}): {1}", error, description);
         });
      }

      {
         PKZL_PROFILE_SCOPE("glfwCreateWindow")
            const auto monitor = m_Settings.IsFullScreen ? glfwGetPrimaryMonitor() : nullptr;

         int clientAPI = GLFW_NO_API;
         if (RenderCore::GetAPI() == RenderCore::API::OpenGL) {
            clientAPI = GLFW_OPENGL_API;
#if defined(PKZL_DEBUG)
            glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
#endif
         }
         glfwWindowHint(GLFW_CLIENT_API, clientAPI);
         glfwWindowHint(GLFW_RESIZABLE, m_Settings.IsResizable ? GLFW_TRUE : GLFW_FALSE);
         m_Window = glfwCreateWindow((int)m_Settings.Width, (int)m_Settings.Height, m_Settings.Title, monitor, nullptr);
         if (!m_Window) {
            throw std::runtime_error("failed to create window");
         }
         glfwSetWindowSizeLimits(
            m_Window,
            m_Settings.MinWidth ? (int)m_Settings.MinWidth : GLFW_DONT_CARE,
            m_Settings.MinHeight ? (int)m_Settings.MinHeight : GLFW_DONT_CARE,
            m_Settings.MaxWidth ? (int)m_Settings.MaxWidth : GLFW_DONT_CARE,
            m_Settings.MaxHeight ? (int)m_Settings.MaxHeight : GLFW_DONT_CARE
         );
         ++s_GLFWWindowCount;
      }

      glfwMakeContextCurrent(m_Window);
      RenderCore::Init();

      m_Context = RenderCore::CreateGraphicsContext(*this);

      SetVSync(true);

      glfwSetWindowSizeCallback(m_Window, [](GLFWwindow* window, int width, int height) {
         EventDispatcher::Send<WindowResizeEvent>(window, width, height);
      });

      glfwSetWindowCloseCallback(m_Window, [](GLFWwindow* window) {
         EventDispatcher::Send<WindowCloseEvent>(window);
      });

      glfwSetKeyCallback(m_Window, [] (GLFWwindow* window, const int key, const int scancode, const int action, const int mods) {
         switch (action) {
            case GLFW_PRESS: {
               EventDispatcher::Send<KeyPressedEvent>(window, key, 0);
               break;
            }
            case GLFW_RELEASE: {
               EventDispatcher::Send<KeyReleasedEvent>(window, key);
               break;
            }
            case GLFW_REPEAT: {
               EventDispatcher::Send<KeyPressedEvent>(window, key, 1);
               break;
            }
         }
      });

      glfwSetCharCallback(m_Window, [] (GLFWwindow* window, unsigned int keycode) {
         EventDispatcher::Send<KeyTypedEvent>(window, (int)keycode);
      });

      glfwSetCursorPosCallback(m_Window, [] (GLFWwindow* window, const double xpos, const double ypos) {
         EventDispatcher::Send<MouseMovedEvent>(window, (float)xpos, (float)ypos);
      });

      glfwSetMouseButtonCallback(m_Window, [] (GLFWwindow* window, const int button, const int action, const int mods) {
         switch (action) {
            case GLFW_PRESS: {
               EventDispatcher::Send<MouseButtonPressedEvent>(window, button);
               break;
            }
            case GLFW_RELEASE: {
               EventDispatcher::Send<MouseButtonReleasedEvent>(window, button);
               break;
            }
         }
      });

      glfwSetScrollCallback(m_Window, [](GLFWwindow * window, double xoffset, double yoffset) {
         EventDispatcher::Send<MouseScrolledEvent>(window, (float)xoffset, (float)yoffset);
      });
   }


   WindowsWindow::~WindowsWindow() {
      glfwDestroyWindow(m_Window);
      m_Window = nullptr;
      if (--s_GLFWWindowCount == 0) {
         glfwTerminate();
      }
   }


   void* WindowsWindow::GetNativeWindow() const {
      return m_Window;
   }


   uint32_t WindowsWindow::GetWidth() const {
      return m_Settings.Width;
   }


   uint32_t WindowsWindow::GetHeight() const {
      return m_Settings.Height;
   }


   void WindowsWindow::SetVSync(bool enabled) {
      if (enabled) {
         glfwSwapInterval(1);
      } else {
         glfwSwapInterval(0);
      }
      m_VSync = enabled;
   }


   bool WindowsWindow::IsVSync() const {
      return m_VSync;
   }


   float WindowsWindow::ContentScale() const {
      float xscale;
      float yscale;
      glfwGetWindowContentScale(m_Window, &xscale, &yscale);
      return xscale;
   }


   void WindowsWindow::BeginFrame() {
      glfwPollEvents();
      m_Context->BeginFrame();
   }


   void WindowsWindow::EndFrame() {
      m_Context->EndFrame();     // i.e. "submit"
      m_Context->SwapBuffers();  // i.e. "present"
   }

}
