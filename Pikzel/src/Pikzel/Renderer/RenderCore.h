#pragma once

#include <memory>

namespace Pikzel {

   class GraphicsContext;
   class Window;

   class RenderCore {
   public:
      virtual ~RenderCore() = default;

      virtual std::unique_ptr<GraphicsContext> CreateGraphicsContext(const Window& window) = 0;

      virtual void BeginFrame() = 0;
      virtual void EndFrame() = 0;

   public:
      static std::unique_ptr<RenderCore> Create();
   };

}
