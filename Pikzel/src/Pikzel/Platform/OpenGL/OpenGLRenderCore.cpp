#include "OpenGLRenderCore.h"
#include "OpenGLBuffer.h"
#include "OpenGLGraphicsContext.h"
#include "OpenGLPipeline.h"
#include "OpenGLTexture.h"

namespace Pikzel {

   RenderCore::API RenderCore::s_API = RenderCore::API::OpenGL;

   std::unique_ptr<IRenderCore> CreateRenderCore(const Window& window) {
      return std::make_unique<OpenGLRenderCore>(window);
   }


   void OpenGLMessageCallback(
      unsigned source,
      unsigned type,
      unsigned id,
      unsigned severity,
      int length,
      const char* message,
      const void* userParam
   ) {
      switch (severity) {
         case GL_DEBUG_SEVERITY_NOTIFICATION:
            PKZL_CORE_LOG_INFO("OpenGL Debug: {0}", message);
            break;
         case GL_DEBUG_SEVERITY_LOW:
            PKZL_CORE_LOG_WARN("OpenGL Debug: {0}", message);
            break;
         case GL_DEBUG_SEVERITY_MEDIUM:
            PKZL_CORE_LOG_ERROR("OpenGL Debug: {0}", message);
            break;
         case GL_DEBUG_SEVERITY_HIGH:
            PKZL_CORE_LOG_FATAL("OpenGL Debug: {0}", message);
            break;
      }
      PKZL_CORE_ASSERT(false, "Unknown OpenGL message callback severity level!");
   }


   OpenGLRenderCore::OpenGLRenderCore(const Window& window) {
      glfwMakeContextCurrent(static_cast<GLFWwindow*>(window.GetNativeWindow()));

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

      glEnable(GL_CULL_FACE);
      glFrontFace(GL_CCW);
      glEnable(GL_DEPTH_TEST);
   }


   OpenGLRenderCore::~OpenGLRenderCore() {}


   void OpenGLRenderCore::SetViewport(const uint32_t x, const uint32_t y, const uint32_t width, const uint32_t height) {
      glViewport(x, y, width, height);
   }


   std::unique_ptr<GraphicsContext> OpenGLRenderCore::CreateGraphicsContext(const Window& window) {
      return std::make_unique<OpenGLGraphicsContext>(window);
   }


   std::unique_ptr<VertexBuffer> OpenGLRenderCore::CreateVertexBuffer(const uint32_t size) {
      return std::make_unique<OpenGLVertexBuffer>(size);
   }


   std::unique_ptr<VertexBuffer> OpenGLRenderCore::CreateVertexBuffer(const uint32_t size, const void* data) {
      return std::make_unique<OpenGLVertexBuffer>(size, data);
   }


   std::unique_ptr<IndexBuffer> OpenGLRenderCore::CreateIndexBuffer(const uint32_t count, const uint32_t* indices) {
      return std::make_unique<OpenGLIndexBuffer>(count, indices);
   }


   std::unique_ptr<UniformBuffer> OpenGLRenderCore::CreateUniformBuffer(const uint32_t size) {
      return std::make_unique<OpenGLUniformBuffer>(size);
   }


   std::unique_ptr<UniformBuffer> OpenGLRenderCore::CreateUniformBuffer(const uint32_t size, const void* data) {
      return std::make_unique<OpenGLUniformBuffer>(size, data);
   }


   bool OpenGLRenderCore::FlipUV() const {
      return false;
   }


   std::unique_ptr<Texture2D> OpenGLRenderCore::CreateTexture2D(const uint32_t width, const uint32_t height) {
      return std::make_unique<OpenGLTexture2D>(width, height);
   }


   std::unique_ptr<Texture2D> OpenGLRenderCore::CreateTexture2D(const std::filesystem::path& path) {
      return std::make_unique<OpenGLTexture2D>(path);
   }

}
