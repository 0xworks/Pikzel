#include "pch.h"
#include "Renderer.h"
#include "GraphicsContext.h"
#include "RenderCore.h"

#include <memory>

namespace Pikzel {

   namespace Renderer {

      static std::unique_ptr<RenderCore> g_RenderCore;

      void Init() {
         g_RenderCore = RenderCore::Create();
      }

      void Shutdown() {
         g_RenderCore.reset();
      }

      RendererAPI GetAPI() {
         return g_RenderCore->GetAPI();
      }

      std::unique_ptr<Pikzel::GraphicsContext> CreateGraphicsContext(const Window& window) {
         PKZL_CORE_ASSERT(g_RenderCore, "g_RenderCore used before Init()!");
         return g_RenderCore->CreateGraphicsContext(window);
      }

   }
}
