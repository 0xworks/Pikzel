#include "Pikzel/Platform/Window.h"
#include "Pikzel/Core/Log.h"

#if defined(_WIN32)
   #define GLFW_EXPOSE_NATIVE_WIN32
#elif defined(__linux__)
   #define GLFW_EXPOSE_NATIVE_X11
   #define GLFW_EXPOSE_NATIVE_WAYLAND
#endif
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#include <cassert>
#include <format>
#include <stdexcept>

namespace Pikzel {

namespace {

// Number of live windows. GLFW is initialised when this goes from 0 to 1 and terminated
// when it returns to 0. Only touched on the main thread, so no synchronisation is needed.
int s_WindowCount = 0;


void GlfwErrorCallback(const int error, const char* description) {
   PKZL_CORE_LOG_ERROR("GLFW error {}: {}", error, description);
}

}


Window::Window(const WindowSettings& settings) {
   if (s_WindowCount == 0) {
      glfwSetErrorCallback(GlfwErrorCallback);
      if (!glfwInit()) {
         throw std::runtime_error("Failed to initialise GLFW");
      }
   }

   glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
   glfwWindowHint(GLFW_RESIZABLE, settings.isResizable ? GLFW_TRUE : GLFW_FALSE);

   m_Window = glfwCreateWindow(static_cast<int>(settings.width), static_cast<int>(settings.height), settings.title.c_str(), nullptr, nullptr);
   if (!m_Window) {
      if (s_WindowCount == 0) {
         glfwTerminate();
      }
      throw std::runtime_error(std::format("Failed to create window '{}'", settings.title));
   }

   ++s_WindowCount;
   PKZL_CORE_LOG_DEBUG("Created window '{}' ({}x{})", settings.title, settings.width, settings.height);
}


Window::~Window() {
   glfwDestroyWindow(m_Window);
   if (--s_WindowCount == 0) {
      glfwTerminate();
   }
}


void Window::PollEvents() {
   glfwPollEvents();
}


void Window::WaitEvents() {
   glfwWaitEvents();
}


bool Window::ShouldClose() const {
   return glfwWindowShouldClose(m_Window);
}


FramebufferSize Window::GetFramebufferSize() const {
   int width = 0;
   int height = 0;
   glfwGetFramebufferSize(m_Window, &width, &height);
   return {static_cast<uint32_t>(width), static_cast<uint32_t>(height)};
}


bool Window::IsMinimized() const {
   const auto size = GetFramebufferSize();
   return size.width == 0 || size.height == 0;
}


NativeWindowHandle Window::GetNativeHandle() const {
#if defined(_WIN32)
   return Win32WindowHandle{glfwGetWin32Window(m_Window)};
#elif defined(__linux__)
   // A Linux build of GLFW can support both X11 and Wayland; which one is in use is
   // decided at run time.
   if (glfwGetPlatform() == GLFW_PLATFORM_WAYLAND) {
      return WaylandWindowHandle{glfwGetWaylandDisplay(), glfwGetWaylandWindow(m_Window)};
   }
   return X11WindowHandle{glfwGetX11Display(), static_cast<uint64_t>(glfwGetX11Window(m_Window))};
#else
   #error "Window::GetNativeHandle() is not implemented for this platform"
#endif
}

}
