#include "pch.h"
#include "WindowsWindow.h"

#include "Pikzel/Events/Events.h"
#include "Pikzel/Events/KeyEvents.h"
#include "Pikzel/Events/MouseEvents.h"
#include "Pikzel/Events/WindowEvents.h"

//#include "Platform/OpenGL/OpenGLContext.h"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

namespace Pikzel {

static bool s_GLFWInitialized = false;


static void GLFWErrorCallback(int error, const char* description) {
   PKZL_CORE_LOG_ERROR("GLFW Error ({0}): {1}", error, description);
}


void GLFWWindowSizeCallback(GLFWwindow* window, int width, int height) {
   Events::SendEvent<WindowResizeEvent>(width, height);
};


void GLFWWindowCloseCallback(GLFWwindow* window) {
   Events::SendEvent<WindowCloseEvent>();
};


void GLFWKeyCallback(GLFWwindow* window, const int key, const int scancode, const int action, const int mods) {
   switch (action) {
      case GLFW_PRESS: {
         Events::SendEvent<KeyPressedEvent>(key, 0);
         break;
      }
      case GLFW_RELEASE: {
         Events::SendEvent<KeyReleasedEvent>(key);
         break;
      }
      case GLFW_REPEAT: {
         Events::SendEvent<KeyPressedEvent>(key, 1);
         break;
      }
   }
}


void GLFWCharCallback(GLFWwindow* window, unsigned int keycode) {
   Events::SendEvent<KeyTypedEvent>((int)keycode);
}


void GLFWCursorPosCallback(GLFWwindow* window, const double xpos, const double ypos) {
   Events::SendEvent<MouseMovedEvent>((float)xpos, (float)ypos);
}


void GLFWMouseButtonCallback(GLFWwindow* window, const int button, const int action, const int mods) {
   switch (action) {
      case GLFW_PRESS: {
         Events::SendEvent<MouseButtonPressedEvent>(button);
         break;
      }
      case GLFW_RELEASE: {
         Events::SendEvent<MouseButtonReleasedEvent>(button);
         break;
      }
   }
}


void GLFWScrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
   Events::SendEvent<MouseScrolledEvent>((float)xoffset, (float)yoffset);
};


std::unique_ptr<Window> Window::Create(const WindowSettings& settings) {
   return std::make_unique<WindowsWindow>(settings);
}


WindowsWindow::WindowsWindow(const WindowSettings& settings) {
   m_Settings = settings;

   PKZL_CORE_LOG_INFO("Creating window {0} ({1}, {2})", m_Settings.Title, m_Settings.Width, m_Settings.Height);

   if (!s_GLFWInitialized) {
      // TODO: glfwTerminate on system shutdown
      int success = glfwInit();
      PKZL_CORE_ASSERT(success, "Could not intialize GLFW!");
      glfwSetErrorCallback(GLFWErrorCallback);
      s_GLFWInitialized = true;
   }

   //glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
   glfwWindowHint(GLFW_RESIZABLE, m_Settings.IsResizable ? GLFW_TRUE : GLFW_FALSE);

   const auto monitor = m_Settings.IsFullScreen ? glfwGetPrimaryMonitor() : nullptr;

   m_Window = glfwCreateWindow((int)m_Settings.Width, (int)m_Settings.Height, m_Settings.Title, monitor, nullptr);
   if (!m_Window) {
      throw std::runtime_error("failed to create window");
   }

   //m_Context = std::make_unique<OpenGLContext>(m_Window);

   SetVSync(true);

   glfwSetWindowUserPointer(m_Window, this);

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


void WindowsWindow::OnUpdate() {
   glfwPollEvents();
   //m_Context->SwapBuffers();
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


void* WindowsWindow::GetNativeWindow() const {
   return m_Window;
}

}
