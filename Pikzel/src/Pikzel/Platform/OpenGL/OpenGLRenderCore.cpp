#include "OpenGLRenderCore.h"
#include "OpenGLBuffer.h"
#include "OpenGLComputeContext.h"
#include "OpenGLGraphicsContext.h"
#include "OpenGLPipeline.h"
#include "OpenGLTexture.h"

#include <glm/ext/matrix_transform.hpp>

#if defined(PKZL_PLATFORM_WINDOWS)
   #define PLATFORM_API __declspec(dllexport)
#else
   #define PLATFORM_API
#endif

namespace Pikzel {

   extern "C" PLATFORM_API IRenderCore* CDECL CreateRenderCore(const Window* window) {
      PKZL_CORE_ASSERT(window, "Window is null in call to CreateRenderCore!");
      return new OpenGLRenderCore {*window};
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
         default:
            PKZL_CORE_LOG_ERROR("OpenGL Debug: The following message was received with severity level {0}.  This is not a known OpenGL callback severity level!", severity);
            PKZL_CORE_LOG_ERROR("OpenGL Debug: {0}", message);
      }
   }


   OpenGLRenderCore::OpenGLRenderCore(const Window& window) {
      glfwMakeContextCurrent(static_cast<GLFWwindow*>(window.GetNativeWindow()));

      if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
         throw std::runtime_error {"Failed to initialize Glad!"};
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

      glClipControl(GL_LOWER_LEFT, GL_ZERO_TO_ONE);

      glEnable(GL_BLEND);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

      glEnable(GL_CULL_FACE);
      glFrontFace(GL_CCW);

      glEnable(GL_DEPTH_TEST);
      //glDepthFunc(GL_LEQUAL);
      glDepthFunc(GL_GEQUAL);  // reverse-Z!

      glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

      glEnable(GL_MULTISAMPLE);
      glEnable(GL_FRAMEBUFFER_SRGB);
   }


   OpenGLRenderCore::~OpenGLRenderCore() {}


   void OpenGLRenderCore::UploadImGuiFonts() {}


   void OpenGLRenderCore::SetViewport(const uint32_t x, const uint32_t y, const uint32_t width, const uint32_t height) {
      glViewport(x, y, width, height);
   }


   std::unique_ptr<ComputeContext> OpenGLRenderCore::CreateComputeContext() {
      return std::make_unique<OpenGLComputeContext>();
   }


   std::unique_ptr<GraphicsContext> OpenGLRenderCore::CreateGraphicsContext(const Window& window) {
      return std::make_unique<OpenGLWindowGC>(window);
   }


   std::unique_ptr<VertexBuffer> OpenGLRenderCore::CreateVertexBuffer(const BufferLayout& layout, const uint32_t size) {
      return std::make_unique<OpenGLVertexBuffer>(layout, size);
   }


   std::unique_ptr<VertexBuffer> OpenGLRenderCore::CreateVertexBuffer(const BufferLayout& layout, const uint32_t size, const void* data) {
      return std::make_unique<OpenGLVertexBuffer>(layout, size, data);
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


   std::unique_ptr<Framebuffer> OpenGLRenderCore::CreateFramebuffer(const FramebufferSettings& settings) {
      return std::make_unique<OpenGLFramebuffer>(settings);
   }


   std::unique_ptr<Texture> OpenGLRenderCore::CreateTexture(const TextureSettings& settings) {
      switch (settings.textureType) {
         case TextureType::Texture2D: return std::make_unique<OpenGLTexture2D>(settings);
         case TextureType::Texture2DArray: return std::make_unique<OpenGLTexture2DArray>(settings);
         case TextureType::TextureCube: return std::make_unique<OpenGLTextureCube>(settings);
         case TextureType::TextureCubeArray: return std::make_unique<OpenGLTextureCubeArray>(settings);
      }
      PKZL_CORE_ASSERT(false, "TextureType not supported!");
      return nullptr;
   }

}
