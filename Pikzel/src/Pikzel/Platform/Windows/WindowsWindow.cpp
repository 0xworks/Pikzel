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

   void GLFWWindowSizeCallback(GLFWwindow* window, int width, int height) {
      EventDispatcher::Send<WindowResizeEvent>(window, width, height);
   };


   void GLFWWindowCloseCallback(GLFWwindow* window) {
      EventDispatcher::Send<WindowCloseEvent>(window);
   };


   void GLFWKeyCallback(GLFWwindow* window, const int key, const int scancode, const int action, const int mods) {
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
   }


   void GLFWCharCallback(GLFWwindow* window, unsigned int keycode) {
      EventDispatcher::Send<KeyTypedEvent>(window, (int)keycode);
   }


   void GLFWCursorPosCallback(GLFWwindow* window, const double xpos, const double ypos) {
      EventDispatcher::Send<MouseMovedEvent>(window, (float)xpos, (float)ypos);
   }


   void GLFWMouseButtonCallback(GLFWwindow* window, const int button, const int action, const int mods) {
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
   }


   void GLFWScrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
      EventDispatcher::Send<MouseScrolledEvent>(window, (float)xoffset, (float)yoffset);
   };


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

      m_Window = glfwCreateWindow((int)m_Settings.Width, (int)m_Settings.Height, m_Settings.Title, monitor, nullptr);
      if (!m_Window) {
         throw std::runtime_error("failed to create window");
      }

      m_Context = Renderer::CreateGraphicsContext(*this);

      SetVSync(true);

      glfwSetWindowSizeCallback(m_Window, GLFWWindowSizeCallback);
      glfwSetWindowCloseCallback(m_Window, GLFWWindowCloseCallback);
      glfwSetKeyCallback(m_Window, GLFWKeyCallback);
      glfwSetCharCallback(m_Window, GLFWCharCallback);
      glfwSetCursorPosCallback(m_Window, GLFWCursorPosCallback);
      glfwSetMouseButtonCallback(m_Window, GLFWMouseButtonCallback);
      glfwSetScrollCallback(m_Window, GLFWScrollCallback);
   }


   WindowsWindow::~WindowsWindow() {
      glfwDestroyWindow(m_Window);
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

   void WindowsWindow::Update() {
      glfwPollEvents();
      m_Context->SwapBuffers();
   }

}
