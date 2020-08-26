#include "glpch.h"
#include "OpenGLRenderCore.h"
#include "OpenGLGraphicsContext.h"

#include <glad/glad.h>

namespace Pikzel {

   std::unique_ptr<IRenderCore> Create() {
      return std::make_unique<OpenGLRenderCore>();
   }


   void OpenGLMessageCallback(
      unsigned source,
      unsigned type,
      unsigned id,
      unsigned severity,
      int length,
      const char* message,
      const void* userParam) {
      switch (severity) {
         case GL_DEBUG_SEVERITY_HIGH:         PKZL_CORE_LOG_FATAL(message); return;
         case GL_DEBUG_SEVERITY_MEDIUM:       PKZL_CORE_LOG_ERROR(message); return;
         case GL_DEBUG_SEVERITY_LOW:          PKZL_CORE_LOG_WARN(message); return;
         case GL_DEBUG_SEVERITY_NOTIFICATION: PKZL_CORE_LOG_TRACE(message); return;
      }
      PKZL_CORE_ASSERT(false, "Unknown OpenGL message callback severity level!");
   }


   OpenGLRenderCore::OpenGLRenderCore() {
      PKZL_PROFILE_FUNCTION();

      if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
         throw std::runtime_error("Failed to initialize Glad!");
      }

      PKZL_CORE_LOG_INFO("OpenGL Info:");
      PKZL_CORE_LOG_INFO("  Vendor: {0}", glGetString(GL_VENDOR));
      PKZL_CORE_LOG_INFO("  Renderer: {0}", glGetString(GL_RENDERER));
      PKZL_CORE_LOG_INFO("  Version: {0}", glGetString(GL_VERSION));

#ifdef PKZL_ENABLE_ASSERTS
      int versionMajor;
      int versionMinor;
      glGetIntegerv(GL_MAJOR_VERSION, &versionMajor);
      glGetIntegerv(GL_MINOR_VERSION, &versionMinor);
      PKZL_CORE_ASSERT(versionMajor > 4 || (versionMajor == 4 && versionMinor >= 5), "Pikzel requires at least OpenGL version 4.5!");
#endif

#ifdef PKZL_DEBUG
      glEnable(GL_DEBUG_OUTPUT);
      glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
      glDebugMessageCallback(OpenGLMessageCallback, nullptr);
      glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION, 0, NULL, GL_FALSE);
#endif

      glEnable(GL_BLEND);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

      glEnable(GL_DEPTH_TEST);
   }


   OpenGLRenderCore::~OpenGLRenderCore() {}


   void OpenGLRenderCore::SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) {
      glViewport(x, y, width, height);
   }


   void OpenGLRenderCore::SetClearColor(const glm::vec4& color) {
      glClearColor(color.r, color.g, color.b, color.a);
   }


   void OpenGLRenderCore::Clear() {
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
   }


   std::unique_ptr<Pikzel::GraphicsContext> OpenGLRenderCore::CreateGraphicsContext(Window& window) {
      return std::make_unique<OpenGLGraphicsContext>(window);
   }

}
