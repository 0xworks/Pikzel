#pragma once

#include "RendererAPI.h"

namespace Pikzel {

   class GraphicsContext;
   class Window;

   namespace Renderer {

      void Init();
      void Shutdown();

      RendererAPI GetAPI();

      std::unique_ptr<GraphicsContext> CreateGraphicsContext(const Window& window);
   }
}
