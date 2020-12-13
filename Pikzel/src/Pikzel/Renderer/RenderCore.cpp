#include "RenderCore.h"
#include "MeshRenderer.h"

namespace Pikzel {

#ifdef PKZL_PLATFORM_WINDOWS
   HINSTANCE gAPILib;
#endif

   void RenderCore::SetAPI(API api) {
      if (s_API != API::Undefined) {
         throw std::logic_error {"RenderCore::SetAPI() can only be called once!"};
      }
      if (api == API::Undefined) {
         throw std::invalid_argument {"RenderCore::SetAPI() cannot be called with API::Undefined!"};
      }
      s_API = api;
      switch (s_API) {
         case API::OpenGL: {
#ifdef PKZL_PLATFORM_WINDOWS
            gAPILib = LoadLibrary("PlatformOpenGL.dll");
            if (gAPILib) {
               CreateRenderCore = (RENDERCORECREATEPROC)GetProcAddress(gAPILib, "CreateRenderCore");
               MeshRenderer::CreateMeshRenderer = (MeshRenderer::MESHRENDERERCREATEPROC)GetProcAddress(gAPILib, "CreateMeshRenderer");
            }
#else
            PKZL_CORE_ASSERT(false, "You havent written non-windows shared library loading code, yet");
#endif
            break;
         }
         case API::Vulkan: {
#ifdef PKZL_PLATFORM_WINDOWS
            gAPILib = LoadLibrary("PlatformVulkan.dll");
            if (gAPILib) {
               CreateRenderCore = (RENDERCORECREATEPROC)GetProcAddress(gAPILib, "CreateRenderCore");
               MeshRenderer::CreateMeshRenderer = (MeshRenderer::MESHRENDERERCREATEPROC)GetProcAddress(gAPILib, "CreateMeshRenderer");
            }
#else
            PKZL_CORE_ASSERT(false, "You havent written non-windows shared library loading code, yet");
#endif
            break;
         }
         default:
            PKZL_CORE_ASSERT(false, "Unknown RenderCore::API"); // you added an API and forgot to write the loading code!
      }
      if (!CreateRenderCore) {
         throw std::runtime_error {"RenderCore api could not be loaded: failed to locate CreateRenderCore proc!"};
      }
      if (!MeshRenderer::CreateMeshRenderer) {
         throw std::runtime_error {"RenderCore api could not be loaded: failed to locate CreateMeshRenderer proc!"};
      }
   }


   RenderCore::API RenderCore::GetAPI() {
      if (s_API == API::Undefined) {
         SetAPI(API::OpenGL);
      }
      return s_API;
   }


   void RenderCore::Init(const Window& window) {
      // Not having a CreateRenderCore proc here is fatal.
      // It's too late to get one by just calling SetAPI(OpenGL) - because by the time you get here, other stuff (like glfw) needs to
      // have already been set up with the correct API.
      PKZL_CORE_ASSERT(CreateRenderCore, "CreateRenderCore is null in call to Init().  Did you call RenderCore::SetAPI()?");
      s_RenderCore.reset(CreateRenderCore(&window));
   }


   void RenderCore::DeInit() {
      s_RenderCore.reset(nullptr);
#ifdef PKZL_PLATFORM_WINDOWS
      FreeLibrary(gAPILib);
#endif
   }


   void RenderCore::UploadImGuiFonts() {
      s_RenderCore->UploadImGuiFonts();
   }


   void RenderCore::SetViewport(const uint32_t x, const uint32_t y, const uint32_t width, const uint32_t height) {
      s_RenderCore->SetViewport(x, y, width, height);
   }


   std::unique_ptr<GraphicsContext> RenderCore::CreateGraphicsContext(const Window& window) {
      return s_RenderCore->CreateGraphicsContext(window);
   }


   std::unique_ptr<VertexBuffer> RenderCore::CreateVertexBuffer(const uint32_t size) {
      return s_RenderCore->CreateVertexBuffer(size);
   }


   std::unique_ptr<VertexBuffer> RenderCore::CreateVertexBuffer(const uint32_t size, const void* data) {
      return s_RenderCore->CreateVertexBuffer(size, data);
   }


   std::unique_ptr<IndexBuffer> RenderCore::CreateIndexBuffer(const uint32_t count, const uint32_t* indices) {
      return s_RenderCore->CreateIndexBuffer(count, indices);
   }


   std::unique_ptr<Pikzel::UniformBuffer> RenderCore::CreateUniformBuffer(const uint32_t size) {
      return s_RenderCore->CreateUniformBuffer(size);
   }


   std::unique_ptr<Pikzel::UniformBuffer> RenderCore::CreateUniformBuffer(const uint32_t size, const void* data) {
      return s_RenderCore->CreateUniformBuffer(size, data);
   }


   std::unique_ptr<Pikzel::Framebuffer> RenderCore::CreateFramebuffer(const FramebufferSettings& settings) {
      if(!(
         (settings.MSAANumSamples == 1) ||
         (settings.MSAANumSamples == 2) ||
         (settings.MSAANumSamples == 4) ||
         (settings.MSAANumSamples == 8)
      )) {
         throw std::runtime_error {"Invalid MSAA sample count.  Must be 1, 2, 4, or 8"};
      }
      return s_RenderCore->CreateFramebuffer(settings);
   }

   std::unique_ptr<Texture> RenderCore::CreateTexture2D(const uint32_t width, const uint32_t height, const TextureFormat format, const uint32_t mipLevels) {
      return s_RenderCore->CreateTexture2D(width, height, format, mipLevels);
   }


   std::unique_ptr<Texture> RenderCore::CreateTexture2D(const std::filesystem::path& path, const bool isSRGB) {
      return s_RenderCore->CreateTexture2D(path, isSRGB);
   }


   std::unique_ptr<Texture> RenderCore::CreateTextureCube(const uint32_t size, TextureFormat format, const uint32_t mipLevels) {
      return s_RenderCore->CreateTextureCube(size, format, mipLevels);
   }


   std::unique_ptr<Texture> RenderCore::CreateTextureCube(const std::filesystem::path& path, const bool isSRGB) {
      return s_RenderCore->CreateTextureCube(path, isSRGB);
   }

}
