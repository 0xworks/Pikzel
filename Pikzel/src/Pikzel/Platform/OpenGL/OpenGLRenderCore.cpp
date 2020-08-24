#include "glpch.h"
#include "OpenGLRenderCore.h"
#include "OpenGLGraphicsContext.h"

#include "Pikzel/Core/Window.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <examples/imgui_impl_glfw.h>
#include <examples/imgui_impl_opengl3.h>

namespace Pikzel {

   std::unique_ptr<RenderCore> RenderCore::Create() {
      return std::make_unique<OpenGLRenderCore>();
   }


   OpenGLRenderCore::OpenGLRenderCore() {
      if (!glfwInit()) {
         throw std::runtime_error("Could not initialize GLFW!");
      }
      glfwSetErrorCallback([] (int error, const char* description) {
         PKZL_CORE_LOG_ERROR("GLFW Error ({0}): {1}", error, description);
      });
   }


   OpenGLRenderCore::~OpenGLRenderCore() {
      glfwTerminate();
   }


   RendererAPI OpenGLRenderCore::GetAPI() const {
      return RendererAPI::OpenGL;
   }


   std::unique_ptr<Pikzel::Buffer> OpenGLRenderCore::CreateBuffer(const uint64_t size) {
      throw std::logic_error("The method or operation is not implemented.");
   }


   std::unique_ptr<Pikzel::Image> OpenGLRenderCore::CreateImage(const ImageSettings& settings /*= ImageSettings()*/) {
      throw std::logic_error("The method or operation is not implemented.");
   }


   std::unique_ptr<GraphicsContext> OpenGLRenderCore::CreateGraphicsContext(Window& window) {
      return std::make_unique<OpenGLGraphicsContext>((GLFWwindow*)window.GetNativeWindow());
   }


   std::unique_ptr<Pikzel::GraphicsContext> OpenGLRenderCore::CreateGraphicsContext(Image& window) {
      throw std::logic_error("The method or operation is not implemented.");
   }

}