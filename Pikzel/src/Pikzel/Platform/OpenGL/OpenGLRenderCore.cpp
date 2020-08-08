#include "pch.h"
#include "OpenGLRenderCore.h"
#include "OpenGLGraphicsContext.h"
#include "Pikzel/Core/Core.h"
#include "Pikzel/Core/Window.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <examples/imgui_impl_glfw.h>
#include <examples/imgui_impl_opengl3.h>

namespace Pikzel {

   static void GLFWErrorCallback(int error, const char* description) {
      PKZL_CORE_LOG_ERROR("GLFW Error ({0}): {1}", error, description);
   }


   std::unique_ptr<RenderCore> RenderCore::Create() {
      return std::make_unique<OpenGLRenderCore>();
   }


   OpenGLRenderCore::OpenGLRenderCore() {
      if (!glfwInit()) {
         throw std::runtime_error("Could not initialize GLFW!");
      }
      glfwSetErrorCallback(GLFWErrorCallback);
   }


   OpenGLRenderCore::~OpenGLRenderCore() {
      ImGui_ImplOpenGL3_Shutdown();
      ImGui_ImplGlfw_Shutdown();
      ImGui::DestroyContext();
      glfwTerminate();
   }


   std::unique_ptr<GraphicsContext> OpenGLRenderCore::CreateGraphicsContext(const Window& window) {
      return std::make_unique<OpenGLGraphicsContext>((GLFWwindow*)window.GetNativeWindow());

   }


   void OpenGLRenderCore::BeginFrame() {
      ImGui_ImplOpenGL3_NewFrame();
      ImGui_ImplGlfw_NewFrame();
      ImGui::NewFrame();
   }


   void OpenGLRenderCore::EndFrame() {
      ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

      if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
         GLFWwindow* currentContext = glfwGetCurrentContext();
         ImGui::UpdatePlatformWindows();
         ImGui::RenderPlatformWindowsDefault();
         glfwMakeContextCurrent(currentContext);
      }
   }

}