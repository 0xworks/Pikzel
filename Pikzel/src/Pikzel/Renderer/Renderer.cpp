#include "pch.h"
#include "Renderer.h"
#include "Pikzel/Renderer/RenderCore.h"
#include "Pikzel/Renderer/GraphicsContext.h"

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

      std::unique_ptr<Pikzel::GraphicsContext> CreateGraphicsContext(const Window& window) {
         PKZL_CORE_ASSERT(g_RenderCore, "g_RenderCore used before Init()!");
         return g_RenderCore->CreateGraphicsContext(window);
      }

      void BeginFrame() {
         g_RenderCore->BeginFrame();
      }

      void EndFrame() {
         g_RenderCore->EndFrame();
      }

   }
}
