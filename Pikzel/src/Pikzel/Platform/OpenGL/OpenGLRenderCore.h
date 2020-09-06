#pragma once

#include "Pikzel/Renderer/RenderCore.h"

namespace Pikzel {

   class OpenGLRenderCore : public IRenderCore {
   public:
      OpenGLRenderCore(const Window& window);
      virtual ~OpenGLRenderCore();

      virtual void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) override;

      virtual std::unique_ptr<GraphicsContext> CreateGraphicsContext(const Window& window) override;

      virtual std::unique_ptr<VertexBuffer> CreateVertexBuffer(uint32_t size) override;
      virtual std::unique_ptr<VertexBuffer> CreateVertexBuffer(float* vertices, uint32_t size) override;

      virtual std::unique_ptr<IndexBuffer> CreateIndexBuffer(uint32_t* indices, uint32_t count) override;

      std::unique_ptr<Texture2D> CreateTexture2D(uint32_t width, uint32_t height) override;
      std::unique_ptr<Texture2D> CreateTexture2D(const std::filesystem::path& path) override;

   };

}
