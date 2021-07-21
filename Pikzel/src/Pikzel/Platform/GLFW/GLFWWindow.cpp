#include "GLFWWindow.h"

#include "Pikzel/Events/EventDispatcher.h"
#include "Pikzel/Events/KeyEvents.h"
#include "Pikzel/Events/MouseEvents.h"
#include "Pikzel/Events/WindowEvents.h"
#include "Pikzel/Renderer/RenderCore.h"

#include <GLFW/glfw3.h>

namespace Pikzel {

   std::unique_ptr<Window> Window::Create() {
      return std::make_unique<GLFWWindow>(Settings{});
   }

   std::unique_ptr<Window> Window::Create(const Settings& settings) {
      return std::make_unique<GLFWWindow>(settings);
   }


   static uint8_t s_GLFWWindowCount = 0;


   GLFWWindow::GLFWWindow(const Settings& settings) {
      m_Settings = settings;

      PKZL_CORE_LOG_INFO("Platform GLFW:");
      PKZL_CORE_LOG_INFO("  Title: {0}", m_Settings.title);
      PKZL_CORE_LOG_INFO("  Size: ({0}, {1})", m_Settings.width, m_Settings.height);

      if (s_GLFWWindowCount == 0) {
         if (!glfwInit()) {
            throw std::runtime_error {"Could not initialize GLFW!"};
         }
         glfwSetErrorCallback([] (int error, const char* description) {
            PKZL_CORE_LOG_ERROR("GLFW Error ({0}): {1}", error, description);
         });
      }

      {
         const auto monitor = m_Settings.isFullScreen ? glfwGetPrimaryMonitor() : nullptr;

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
         glfwWindowHint(GLFW_RESIZABLE, m_Settings.isResizable ? GLFW_TRUE : GLFW_FALSE);
         glfwWindowHint(GLFW_MAXIMIZED, m_Settings.isMaximized);

         if (!(
            (settings.msaaNumSamples == 1) ||
            (settings.msaaNumSamples == 2) ||
            (settings.msaaNumSamples == 4) ||
            (settings.msaaNumSamples == 8)
         )) {
            throw std::runtime_error {"Invalid MSAA sample count.  Must be 1, 2, 4, or 8"};
         }
         glfwWindowHint(GLFW_SAMPLES, m_Settings.msaaNumSamples);
         m_Window = glfwCreateWindow((int)m_Settings.width, (int)m_Settings.height, m_Settings.title, monitor, nullptr);
         if (!m_Window) {
            throw std::runtime_error {"failed to create window"};
         }
         glfwSetWindowSizeLimits(
            m_Window,
            m_Settings.minWidth ? (int)m_Settings.minWidth : GLFW_DONT_CARE,
            m_Settings.minHeight ? (int)m_Settings.minHeight : GLFW_DONT_CARE,
            m_Settings.maxWidth ? (int)m_Settings.maxWidth : GLFW_DONT_CARE,
            m_Settings.maxHeight ? (int)m_Settings.maxHeight : GLFW_DONT_CARE
         );
         if (!m_Settings.isCursorEnabled) {
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


   GLFWWindow::~GLFWWindow() {
      m_Context.reset();
      glfwDestroyWindow(m_Window);
      m_Window = nullptr;
      if (--s_GLFWWindowCount == 0) {
         glfwTerminate();
      }
   }


   void* GLFWWindow::GetNativeWindow() const {
      return m_Window;
   }


   uint32_t GLFWWindow::GetWidth() const {
      int width;
      int height;
      glfwGetWindowSize(m_Window, &width, &height);
      return static_cast<uint32_t>(width);
   }


   uint32_t GLFWWindow::GetHeight() const {
      int width;
      int height;
      glfwGetWindowSize(m_Window, &width, &height);
      return static_cast<uint32_t>(height);
   }


   uint32_t GLFWWindow::GetMSAANumSamples() const {
      return m_Settings.msaaNumSamples;
   }


   glm::vec4 GLFWWindow::GetClearColor() const {
      return m_Settings.clearColor;
   }


   void GLFWWindow::SetVSync(bool enabled) {
      m_Settings.isVSync = enabled;
      EventDispatcher::Send<WindowVSyncChangedEvent>(this, enabled);
   }


   bool GLFWWindow::IsVSync() const {
      return m_Settings.isVSync;
   }


   float GLFWWindow::ContentScale() const {
      float xscale;
      float yscale;
      glfwGetWindowContentScale(m_Window, &xscale, &yscale);
      return xscale;
   }


   void GLFWWindow::InitializeImGui() {
      m_Context->InitializeImGui();
   }


   void GLFWWindow::BeginFrame() {
      m_Context->BeginFrame();
   }


   void GLFWWindow::EndFrame() {
      PKZL_PROFILE_FUNCTION();
      m_Context->EndFrame();     // i.e. "submit"
      m_Context->SwapBuffers();  // i.e. "present"
   }


   void GLFWWindow::BeginImGuiFrame() {
      m_Context->BeginImGuiFrame();
   }


   void GLFWWindow::EndImGuiFrame() {
      m_Context->EndImGuiFrame();
   }


   Pikzel::GraphicsContext& GLFWWindow::GetGraphicsContext() {
      PKZL_CORE_ASSERT(m_Context, "Accessing null graphics context!");
      return *m_Context;
   }


   glm::vec2 GLFWWindow::GetCursorPos() const {
      double x;
      double y;
      glfwGetCursorPos(m_Window, &x, &y);
      return {x, y};
   }

}
