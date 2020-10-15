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

      virtual void SetViewport(const uint32_t x, const uint32_t y, const uint32_t width, const uint32_t height) = 0;

      virtual std::unique_ptr<GraphicsContext> CreateGraphicsContext(const Window& window) = 0;

      virtual std::unique_ptr<VertexBuffer> CreateVertexBuffer(const uint32_t size) = 0;
      virtual std::unique_ptr<VertexBuffer> CreateVertexBuffer(const uint32_t size, const void* data) = 0;

      virtual std::unique_ptr<IndexBuffer> CreateIndexBuffer(const uint32_t count, const uint32_t* indices) = 0;

      virtual std::unique_ptr<UniformBuffer> CreateUniformBuffer(const uint32_t size) = 0;
      virtual std::unique_ptr<UniformBuffer> CreateUniformBuffer(const uint32_t size, const void* data) = 0;

      virtual bool FlipUV() const = 0;

      // TODO: you probably also want to be able to specify things like format, and sampling parameters
      virtual std::unique_ptr<Texture2D> CreateTexture2D(const uint32_t width, const uint32_t height) = 0;
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

      static void SetViewport(const uint32_t x, const uint32_t y, const uint32_t width, const uint32_t height);

      static std::unique_ptr<GraphicsContext> CreateGraphicsContext(const Window& window);

      static std::unique_ptr<VertexBuffer> CreateVertexBuffer(const uint32_t size);
      static std::unique_ptr<VertexBuffer> CreateVertexBuffer(const uint32_t size, const void* data);

      static std::unique_ptr<IndexBuffer> CreateIndexBuffer(const uint32_t count, const uint32_t* indices);

      static std::unique_ptr<UniformBuffer> CreateUniformBuffer(const uint32_t size);
      static std::unique_ptr<UniformBuffer> CreateUniformBuffer(const uint32_t size, const void* data);

      static bool FlipUV(); // true if UV coordinates need to be flipped in Y-axis (like, OpenGL vs. Vulkan)
      static std::unique_ptr<Texture2D> CreateTexture2D(const uint32_t width, const uint32_t height);
      static std::unique_ptr<Texture2D> CreateTexture2D(const std::filesystem::path& path);

   private:
      static API s_API;
      static std::unique_ptr<IRenderCore> s_RenderCore;
   };

}
