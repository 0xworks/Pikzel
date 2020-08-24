#include "pch.h"
#include "WindowsWindow.h"

#include "Pikzel/Events/EventDispatcher.h"
#include "Pikzel/Events/KeyEvents.h"
#include "Pikzel/Events/MouseEvents.h"
#include "Pikzel/Events/WindowEvents.h"
#include "Pikzel/Renderer/GraphicsContext.h"
#include "Pikzel/Renderer/Renderer.h"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

namespace Pikzel {

   std::unique_ptr<Window> Window::Create(const WindowSettings& settings) {
      return std::make_unique<WindowsWindow>(settings);
   }


   WindowsWindow::WindowsWindow(const WindowSettings& settings) {
      m_Settings = settings;

      PKZL_CORE_LOG_INFO("Platform Windows:");
      PKZL_CORE_LOG_INFO("  Title: {0}", m_Settings.Title);
      PKZL_CORE_LOG_INFO("  Size: ({0}, {1})", m_Settings.Width, m_Settings.Height);

      glfwWindowHint(GLFW_RESIZABLE, m_Settings.IsResizable ? GLFW_TRUE : GLFW_FALSE);

      const auto monitor = m_Settings.IsFullScreen ? glfwGetPrimaryMonitor() : nullptr;

      int clientAPI = GLFW_NO_API;
      if (Renderer::GetAPI() == RendererAPI::OpenGL) {
         clientAPI = GLFW_OPENGL_API;
      }
      glfwWindowHint(GLFW_CLIENT_API, clientAPI);
      glfwWindowHint(GLFW_RESIZABLE, m_Settings.IsResizable ? GLFW_TRUE : GLFW_FALSE);
      m_Window = glfwCreateWindow((int)m_Settings.Width, (int)m_Settings.Height, m_Settings.Title, monitor, nullptr);
      if (!m_Window) {
         throw std::runtime_error("failed to create window");
      }

      m_Context = Renderer::CreateGraphicsContext(*this);

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
      //if (enabled) {
      //   glfwSwapInterval(1);
      //} else {
      //   glfwSwapInterval(0);
      //}
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


   void WindowsWindow::UploadImGuiFonts() {
      m_Context->UploadImGuiFonts();
   }


   void WindowsWindow::BeginFrame() {
      glfwPollEvents();
      m_Context->BeginFrame();
   }


   void WindowsWindow::EndFrame() {
      m_Context->EndFrame();
      m_Context->SwapBuffers();
   }

   void WindowsWindow::BeginImGuiFrame() {
      m_Context->BeginImGuiFrame();
   }


   void WindowsWindow::EndImGuiFrame() {
      m_Context->EndImGuiFrame();
   }

}
