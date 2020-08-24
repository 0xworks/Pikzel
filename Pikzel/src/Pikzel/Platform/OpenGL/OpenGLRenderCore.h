#pragma once

#include "Pikzel/Renderer/RenderCore.h"

namespace Pikzel {

   class OpenGLRenderCore : public RenderCore {
   public:
      OpenGLRenderCore();
      virtual ~OpenGLRenderCore();

      virtual RendererAPI GetAPI() const override;

      virtual std::unique_ptr<Buffer> CreateBuffer(const uint64_t size) override;
      virtual std::unique_ptr<Image> CreateImage(const ImageSettings& settings = ImageSettings()) override;

      virtual  std::unique_ptr<GraphicsContext> CreateGraphicsContext(Window& window) override;
      virtual  std::unique_ptr<GraphicsContext> CreateGraphicsContext(Image& window) override;



   };

}
