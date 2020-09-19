#include "WindowsWindow.h"

#include "Pikzel/Events/EventDispatcher.h"
#include "Pikzel/Events/KeyEvents.h"
#include "Pikzel/Events/MouseEvents.h"
#include "Pikzel/Events/WindowEvents.h"
#include "Pikzel/Renderer/RenderCore.h"

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
         if (!glfwInit()) {
            throw std::runtime_error("Could not initialize GLFW!");
         }
         glfwSetErrorCallback([] (int error, const char* description) {
            PKZL_CORE_LOG_ERROR("GLFW Error ({0}): {1}", error, description);
         });
      }

      {
         const auto monitor = m_Settings.IsFullScreen ? glfwGetPrimaryMonitor() : nullptr;

         int clientAPI = GLFW_NO_API;
         if (RenderCore::GetAPI() == RenderCore::API::OpenGL) {
            clientAPI = GLFW_OPENGL_API;
#if defined(PKZL_DEBUG)
            glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
#endif
         }
         if (RenderCore::GetAPI() == RenderCore::API::Vulkan) {
            if (!glfwVulkanSupported()) {
               throw std::runtime_error("GLFW detects no support for Vulkan!");
            }
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

      RenderCore::Init(*this);

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


   glm::vec4 WindowsWindow::GetClearColor() const {
      return m_Settings.ClearColor;
   }


   void WindowsWindow::SetVSync(bool enabled) {
      switch (RenderCore::GetAPI()) {
         case RenderCore::API::OpenGL:
            if (enabled) {
               glfwSwapInterval(1);
            } else {
               glfwSwapInterval(0);
            }
            break;
         case RenderCore::API::Vulkan:
            // TODO: support SetVSync on Vulkan... you need to re-create the swapchain with a different present mode (currently hard-coded to "mailbox")
            PKZL_CORE_LOG_WARN("SetVSync() not currently supported on Vulkan - setting ignored");
            break;
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


   Pikzel::GraphicsContext& WindowsWindow::GetGraphicsContext() {
      PKZL_CORE_ASSERT(m_Context, "Accessing null graphics context!");
      return *m_Context;
   }

}
