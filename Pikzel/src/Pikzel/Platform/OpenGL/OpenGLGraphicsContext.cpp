#include "pch.h"
#include "OpenGLGraphicsContext.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <imgui.h>
#include <imgui_internal.h>
#include <examples/imgui_impl_glfw.h>
#include <examples/imgui_impl_opengl3.h>

namespace Pikzel {

   bool OpenGLGraphicsContext::s_OpenGLInitialized = false;

   OpenGLGraphicsContext::OpenGLGraphicsContext(GLFWwindow* window) : m_Window(window) {
      glfwMakeContextCurrent(m_Window);
      if (!s_OpenGLInitialized) {
         if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
            throw std::runtime_error("Failed to initialize Glad!");
         }

         PKZL_CORE_LOG_INFO("Platform OpenGL:");
         PKZL_CORE_LOG_INFO("  Vendor: {0}", glGetString(GL_VENDOR));
         PKZL_CORE_LOG_INFO("  Renderer: {0}", glGetString(GL_RENDERER));
         PKZL_CORE_LOG_INFO("  Version: {0}", glGetString(GL_VERSION));

         int versionMajor;
         int versionMinor;
         glGetIntegerv(GL_MAJOR_VERSION, &versionMajor);
         glGetIntegerv(GL_MINOR_VERSION, &versionMinor);
         if ((versionMajor < 4) || (versionMajor == 4 && versionMinor < 5)) {
            throw std::runtime_error("Pikzel requires at least OpenGL version 4.5!");
         }

         IMGUI_CHECKVERSION();
         ImGui::CreateContext();

         ImGuiIO& io = ImGui::GetIO();
         io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
         io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
         io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

         ImGui_ImplGlfw_InitForOpenGL(m_Window, true);
         ImGui_ImplOpenGL3_Init();

         s_OpenGLInitialized = true;
      }
   }


   void OpenGLGraphicsContext::SwapBuffers() {
      glfwSwapBuffers(m_Window);
   }

}
