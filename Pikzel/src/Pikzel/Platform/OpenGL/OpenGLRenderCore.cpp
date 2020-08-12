#include "glpch.h"
#include "OpenGLRenderCore.h"
#include "OpenGLGraphicsContext.h"

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


   RendererAPI OpenGLRenderCore::GetAPI() const {
      return RendererAPI::OpenGL;
   }


   OpenGLRenderCore::OpenGLRenderCore() {
      PKZL_CORE_LOG_INFO("OpenGL RenderCore");
      if (!glfwInit()) {
         throw std::runtime_error("Could not initialize GLFW!");
      }
      glfwSetErrorCallback(GLFWErrorCallback);
   }


   OpenGLRenderCore::~OpenGLRenderCore() {
      if (ImGui::GetCurrentContext()) {
         ImGui_ImplOpenGL3_Shutdown();
         ImGui_ImplGlfw_Shutdown();
         ImGui::DestroyContext();
      }
      glfwTerminate();
   }


   std::unique_ptr<GraphicsContext> OpenGLRenderCore::CreateGraphicsContext(const Window& window) {
      return std::make_unique<OpenGLGraphicsContext>((GLFWwindow*)window.GetNativeWindow());
   }

}