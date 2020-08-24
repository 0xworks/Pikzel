#include "pch.h"
#include "Renderer.h"
#include "GraphicsContext.h"
#include "RenderCore.h"

#include <memory>

namespace Pikzel {

   namespace Renderer {

      static std::shared_ptr<RenderCore> g_RenderCore;

      void Init() {
         g_RenderCore = RenderCore::Create();
      }

      void Shutdown() {
         g_RenderCore.reset();
      }

      RendererAPI GetAPI() {
         return g_RenderCore->GetAPI();
      }


      std::unique_ptr<Buffer> CreateBuffer(const uint32_t size) {
         PKZL_CORE_ASSERT(g_RenderCore, "g_RenderCore used before Init()!");
         return g_RenderCore->CreateBuffer(size);
      }


      std::unique_ptr<Image> CreateImage(const ImageSettings& settings /* = ImageSettings() */) {
         PKZL_CORE_ASSERT(g_RenderCore, "g_RenderCore used before Init()!");
         return g_RenderCore->CreateImage(settings);
      }


      std::unique_ptr<GraphicsContext> CreateGraphicsContext(Window& window) {
         PKZL_CORE_ASSERT(g_RenderCore, "g_RenderCore used before Init()!");
         return g_RenderCore->CreateGraphicsContext(window);
      }


      std::unique_ptr<GraphicsContext> CreateGraphicsContext(Image& image) {
         PKZL_CORE_ASSERT(g_RenderCore, "g_RenderCore used before Init()!");
         return g_RenderCore->CreateGraphicsContext(image);
      }

   }
}
