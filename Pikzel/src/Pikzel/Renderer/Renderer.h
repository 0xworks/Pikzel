#pragma once

namespace Pikzel {

   class GraphicsContext;
   class Window;

   namespace Renderer {

      void Init();
      void Shutdown();

      std::unique_ptr<GraphicsContext> CreateGraphicsContext(const Window& window);

      void BeginFrame();
      void EndFrame();
   }

}
