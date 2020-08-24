#pragma once

#include "Buffer.h"
#include "Image.h"
#include "RendererAPI.h"
#include <memory>

namespace Pikzel {

   class GraphicsContext;
   class Window;
   class Image;

   class RenderCore {
   public:
      virtual ~RenderCore() = default;

      virtual RendererAPI GetAPI() const = 0;

      virtual std::unique_ptr<Buffer> CreateBuffer(const uint64_t size) = 0;

      virtual std::unique_ptr<Image> CreateImage(const ImageSettings& settings = ImageSettings()) = 0;

      virtual std::unique_ptr<GraphicsContext> CreateGraphicsContext(Window& window) = 0;
      virtual std::unique_ptr<GraphicsContext> CreateGraphicsContext(Image& window) = 0;

   public:
      static std::unique_ptr<RenderCore> Create();
   };

}
