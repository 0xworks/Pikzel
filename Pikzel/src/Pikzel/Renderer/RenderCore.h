#pragma once

#include "Buffer.h"
#include "GraphicsContext.h"
#include "Pipeline.h"
#include "Texture.h"
#include "Pikzel/Core/Window.h"

#include <glm/glm.hpp>

#include <filesystem>
#include <memory>
#include <vector>

namespace Pikzel {

   struct IRenderCore {
      virtual ~IRenderCore() = default;

      virtual void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) = 0;

      virtual std::unique_ptr<GraphicsContext> CreateGraphicsContext(const Window& window) = 0;

      virtual std::unique_ptr<VertexBuffer> CreateVertexBuffer(uint32_t size) = 0;
      virtual std::unique_ptr<VertexBuffer> CreateVertexBuffer(float* vertices, uint32_t size) = 0;

      virtual std::unique_ptr<IndexBuffer> CreateIndexBuffer(uint32_t* indices, uint32_t count) = 0;

      // TODO: you probably also want to be able to specify things like format, and sampling parameters
      virtual std::unique_ptr<Texture2D> CreateTexture2D(uint32_t width, uint32_t height) = 0;
      virtual std::unique_ptr<Texture2D> CreateTexture2D(const std::filesystem::path& path) = 0;

   };


   class RenderCore {
   public:

      enum class API {
         None,
         OpenGL,
         Vulkan
      };

      static API GetAPI();

      static void Init(const Window& window);

      static void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height);

      static std::unique_ptr<GraphicsContext> CreateGraphicsContext(const Window& window);

      static std::unique_ptr<VertexBuffer> CreateVertexBuffer(uint32_t size);
      static std::unique_ptr<VertexBuffer> CreateVertexBuffer(float* vertices, uint32_t size);

      static std::unique_ptr<IndexBuffer> CreateIndexBuffer(uint32_t* indices, uint32_t count);

      static std::unique_ptr<Texture2D> CreateTexture2D(uint32_t width, uint32_t height);
      static std::unique_ptr<Texture2D> CreateTexture2D(const std::filesystem::path& path);

   private:
      static API s_API;
      static std::unique_ptr<IRenderCore> s_RenderCore;
   };

}
