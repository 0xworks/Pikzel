#include "WindowsWindow.h"

#include "Pikzel/Events/EventDispatcher.h"
#include "Pikzel/Events/KeyEvents.h"
#include "Pikzel/Events/MouseEvents.h"
#include "Pikzel/Events/WindowEvents.h"
#include "Pikzel/Renderer/RenderCore.h"

#include <GLFW/glfw3.h>

namespace Pikzel {

   std::unique_ptr<Window> Window::Create(const Settings& settings) {
      return std::make_unique<WindowsWindow>(settings);
   }


   static uint8_t s_GLFWWindowCount = 0;


   WindowsWindow::WindowsWindow(const Settings& settings) {
      m_Settings = settings;

      PKZL_CORE_LOG_INFO("Platform Windows:");
      PKZL_CORE_LOG_INFO("  Title: {0}", m_Settings.Title);
      PKZL_CORE_LOG_INFO("  Size: ({0}, {1})", m_Settings.Width, m_Settings.Height);

      if (s_GLFWWindowCount == 0) {
         if (!glfwInit()) {
            throw std::runtime_error {"Could not initialize GLFW!"};
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
               throw std::runtime_error {"GLFW detects no support for Vulkan!"};
            }
         }
         glfwWindowHint(GLFW_CLIENT_API, clientAPI);
         glfwWindowHint(GLFW_RESIZABLE, m_Settings.IsResizable ? GLFW_TRUE : GLFW_FALSE);

         if ((m_Settings.MSAANumSamples <= 0) || (m_Settings.MSAANumSamples > 64) || (m_Settings.MSAANumSamples & (m_Settings.MSAANumSamples - 1))) {
            throw std::logic_error {"WindowSettings AANumSamples is invalid.  Must be a power of 2, up to 64"};
         }
         glfwWindowHint(GLFW_SAMPLES, m_Settings.MSAANumSamples);
         m_Window = glfwCreateWindow((int)m_Settings.Width, (int)m_Settings.Height, m_Settings.Title, monitor, nullptr);
         if (!m_Window) {
            throw std::runtime_error {"failed to create window"};
         }
         glfwSetWindowSizeLimits(
            m_Window,
            m_Settings.MinWidth ? (int)m_Settings.MinWidth : GLFW_DONT_CARE,
            m_Settings.MinHeight ? (int)m_Settings.MinHeight : GLFW_DONT_CARE,
            m_Settings.MaxWidth ? (int)m_Settings.MaxWidth : GLFW_DONT_CARE,
            m_Settings.MaxHeight ? (int)m_Settings.MaxHeight : GLFW_DONT_CARE
         );
         if (!m_Settings.IsCursorEnabled) {
            glfwSetInputMode(m_Window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
         }
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
               EventDispatcher::Send<KeyPressedEvent>(window, static_cast<KeyCode>(key), 0);
               break;
            }
            case GLFW_RELEASE: {
               EventDispatcher::Send<KeyReleasedEvent>(window, static_cast<KeyCode>(key));
               break;
            }
            case GLFW_REPEAT: {
               EventDispatcher::Send<KeyPressedEvent>(window, static_cast<KeyCode>(key), 1);
               break;
            }
         }
      });

      glfwSetCharCallback(m_Window, [] (GLFWwindow* window, unsigned int key) {
         EventDispatcher::Send<KeyTypedEvent>(window, static_cast<KeyCode>(key));
      });

      glfwSetCursorPosCallback(m_Window, [] (GLFWwindow* window, const double xpos, const double ypos) {
         EventDispatcher::Send<MouseMovedEvent>(window, static_cast<float>(xpos), static_cast<float>(ypos));
      });

      glfwSetMouseButtonCallback(m_Window, [] (GLFWwindow* window, const int button, const int action, const int mods) {
         switch (action) {
            case GLFW_PRESS: {
               EventDispatcher::Send<MouseButtonPressedEvent>(window, static_cast<MouseButton>(button));
               break;
            }
            case GLFW_RELEASE: {
               EventDispatcher::Send<MouseButtonReleasedEvent>(window, static_cast<MouseButton>(button));
               break;
            }
         }
      });

      glfwSetScrollCallback(m_Window, [](GLFWwindow * window, double xoffset, double yoffset) {
         EventDispatcher::Send<MouseScrolledEvent>(window, (float)xoffset, (float)yoffset);
      });
   }


   WindowsWindow::~WindowsWindow() {
      m_Context.reset();
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
      int width;
      int height;
      glfwGetWindowSize(m_Window, &width, &height);
      return static_cast<uint32_t>(width);
   }


   uint32_t WindowsWindow::GetHeight() const {
      int width;
      int height;
      glfwGetWindowSize(m_Window, &width, &height);
      return static_cast<uint32_t>(height);
   }


   uint32_t WindowsWindow::GetMSAANumSamples() const {
      return m_Settings.MSAANumSamples;
   }


   glm::vec4 WindowsWindow::GetClearColor() const {
      return m_Settings.ClearColor;
   }


   void WindowsWindow::SetVSync(bool enabled) {
      m_Settings.IsVSync = enabled;
      EventDispatcher::Send<WindowVSyncChangedEvent>(this, enabled);
   }


   bool WindowsWindow::IsVSync() const {
      return m_Settings.IsVSync;
   }


   float WindowsWindow::ContentScale() const {
      float xscale;
      float yscale;
      glfwGetWindowContentScale(m_Window, &xscale, &yscale);
      return xscale;
   }


   void WindowsWindow::InitializeImGui() {
      m_Context->InitializeImGui();
   }


   void WindowsWindow::BeginFrame() {
      m_Context->BeginFrame();
   }


   void WindowsWindow::EndFrame() {
      m_Context->EndFrame();     // i.e. "submit"
      m_Context->SwapBuffers();  // i.e. "present"
      {
         PKZL_PROFILE_SCOPE("glfwPollEvents");
         glfwPollEvents();
      }
   }


   void WindowsWindow::BeginImGuiFrame() {
      m_Context->BeginImGuiFrame();
   }


   void WindowsWindow::EndImGuiFrame() {
      m_Context->EndImGuiFrame();
   }


   Pikzel::GraphicsContext& WindowsWindow::GetGraphicsContext() {
      PKZL_CORE_ASSERT(m_Context, "Accessing null graphics context!");
      return *m_Context;
   }


   glm::vec2 WindowsWindow::GetCursorPos() const {
      double x;
      double y;
      glfwGetCursorPos(m_Window, &x, &y);
      return {x, y};
   }

}
