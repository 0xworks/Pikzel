#include "pch.h"
#include "Application.h"
#include "Log.h"

#include <GLFW/glfw3.h>

namespace Pikzel {

void glfwErrorCallback(const int error, const char* const description) {
   CORE_LOG_ERROR("GLFW: {0} (code: {1})", description, error);
}


void glfwKeyCallback(GLFWwindow* window, const int key, const int scancode, const int action, const int mods) {
   const auto app = reinterpret_cast<Application*>(glfwGetWindowUserPointer(window));
   app->OnKey(key, scancode, action, mods);
}


void glfwCursorPosCallback(GLFWwindow* window, const double xpos, const double ypos) {
   const auto app = reinterpret_cast<Application*>(glfwGetWindowUserPointer(window));
   app->OnCursorPos(xpos, ypos);
}


void glfwMouseButtonCallback(GLFWwindow* window, const int button, const int action, const int mods) {
   const auto app = reinterpret_cast<Application*>(glfwGetWindowUserPointer(window));
   app->OnMouseButton(button, action, mods);
}

void glfwFramebufferResizeCallback(GLFWwindow* window, const int width, const int height) {
   auto app = reinterpret_cast<Application*>(glfwGetWindowUserPointer(window));
   app->OnWindowResize(width, height);
}


Application::Application(const ApplicationSettings& settings)
: m_Settings(settings)
{}


Application::~Application() {
   // device.WaitIdle();
   DestroyWindow();
}


void Application::Run() {
   glfwSetTime(m_AppTime);
   m_Running = true;
   while (!glfwWindowShouldClose(m_Window) && m_Running) {
      PKZL_PROFILE_FRAMEMARKER();

      glfwPollEvents();

      // TODO: do nothing if window is minimized
      double currentTime = glfwGetTime();
      Update(currentTime - m_AppTime);
      m_AppTime = currentTime;

      Render();
      glfwSwapBuffers(m_Window);
   }
}


void Application::OnKey(const int key, const int scancode, const int action, const int mods) {
}


void Application::OnCursorPos(const double xpos, const double ypos) {
}


void Application::OnMouseButton(const int button, const int action, const int mods) {
   if (button == GLFW_MOUSE_BUTTON_LEFT) {
      if (action == GLFW_PRESS) {
         m_LeftMouseDown = true;
         glfwGetCursorPos(m_Window, &m_MouseX, &m_MouseY);
      } else if (action == GLFW_RELEASE) {
         m_LeftMouseDown = false;
      }
   }
}


void Application::OnWindowResize(const int width, const int height) {
}


void Application::Init() {
   glfwSetErrorCallback(glfwErrorCallback);
   if (!glfwInit()) {
      throw std::runtime_error("glfwInit() failed");
   }
   CreateWindow();
}


void Application::CreateWindow() {
   //glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
   glfwWindowHint(GLFW_RESIZABLE, m_Settings.IsResizable ? GLFW_TRUE : GLFW_FALSE);

   const auto monitor = m_Settings.IsFullScreen ? glfwGetPrimaryMonitor() : nullptr;

   m_Window = glfwCreateWindow(m_Settings.WindowWidth, m_Settings.WindowHeight, m_Settings.ApplicationName, monitor, nullptr);
   if (!m_Window) {
      throw std::runtime_error("failed to create window");
   }

   // TODO: load window icon here...

   if (!m_Settings.IsCursorEnabled) {
      glfwSetInputMode(m_Window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
   }

   glfwSetWindowUserPointer(m_Window, this);
   glfwSetKeyCallback(m_Window, glfwKeyCallback);
   glfwSetCursorPosCallback(m_Window, glfwCursorPosCallback);
   glfwSetMouseButtonCallback(m_Window, glfwMouseButtonCallback);
   glfwSetFramebufferSizeCallback(m_Window, glfwFramebufferResizeCallback);
}


void Application::DestroyWindow() {
   glfwDestroyWindow(m_Window);
   m_Window = nullptr;
}


void Application::Update(double deltaTime) {
}

void Application::Render() {
}


}