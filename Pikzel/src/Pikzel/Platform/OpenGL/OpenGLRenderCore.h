#pragma once

#include "Pikzel/Renderer/RenderCore.h"

namespace Pikzel {

   class OpenGLRenderCore : public IRenderCore {
   public:
      OpenGLRenderCore(const Window& window);
      virtual ~OpenGLRenderCore();

      virtual void UploadImGuiFonts() override;

      virtual void SetViewport(const uint32_t x, const uint32_t y, const uint32_t width, const uint32_t height) override;

      virtual std::unique_ptr<GraphicsContext> CreateGraphicsContext(const Window& window) override;

      virtual std::unique_ptr<VertexBuffer> CreateVertexBuffer(const BufferLayout& layout, const uint32_t size) override;
      virtual std::unique_ptr<VertexBuffer> CreateVertexBuffer(const BufferLayout& layout, const uint32_t size, const void* data) override;

      virtual std::unique_ptr<IndexBuffer> CreateIndexBuffer(const uint32_t count, const uint32_t* indices) override;

      virtual std::unique_ptr<UniformBuffer> CreateUniformBuffer(const uint32_t size) override;
      virtual std::unique_ptr<UniformBuffer> CreateUniformBuffer(const uint32_t size, const void* data) override;

      virtual std::unique_ptr<Framebuffer> CreateFramebuffer(const FramebufferSettings& settings) override;

      virtual std::unique_ptr<Texture> CreateTexture(const TextureSettings& settings) override;

   };

}
