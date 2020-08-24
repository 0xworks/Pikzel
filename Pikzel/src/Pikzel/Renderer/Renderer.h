#pragma once

#include "Buffer.h"
#include "Image.h"
#include "RendererAPI.h"

namespace Pikzel {

   class GraphicsContext;
   class Window;

   namespace Renderer {

      void Init();
      void Shutdown();

      RendererAPI GetAPI();

      std::unique_ptr<Buffer> CreateBuffer(const uint64_t size);
      std::unique_ptr<Image> CreateImage(const ImageSettings& settings = ImageSettings());

      std::unique_ptr<GraphicsContext> CreateGraphicsContext(Window& window);
      std::unique_ptr<GraphicsContext> CreateGraphicsContext(Image& image);
   }
}
