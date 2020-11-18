#pragma once

#include "Buffer.h"
#include "Framebuffer.h"
#include "GraphicsContext.h"
#include "Pipeline.h"
#include "Texture.h"
#include "Pikzel/Core/Window.h"

#include <glm/glm.hpp>

#include <filesystem>
#include <memory>
#include <vector>

namespace Pikzel {

   struct PKZL_API IRenderCore {
      virtual ~IRenderCore() = default;

      virtual void UploadImGuiFonts() = 0;

      virtual void SetViewport(const uint32_t x, const uint32_t y, const uint32_t width, const uint32_t height) = 0;

      virtual std::unique_ptr<GraphicsContext> CreateGraphicsContext(const Window& window) = 0;

      virtual std::unique_ptr<VertexBuffer> CreateVertexBuffer(const uint32_t size) = 0;
      virtual std::unique_ptr<VertexBuffer> CreateVertexBuffer(const uint32_t size, const void* data) = 0;

      virtual std::unique_ptr<IndexBuffer> CreateIndexBuffer(const uint32_t count, const uint32_t* indices) = 0;

      virtual std::unique_ptr<UniformBuffer> CreateUniformBuffer(const uint32_t size) = 0;
      virtual std::unique_ptr<UniformBuffer> CreateUniformBuffer(const uint32_t size, const void* data) = 0;

      virtual std::unique_ptr<Framebuffer> CreateFramebuffer(const FramebufferSettings& settings) = 0;

      virtual std::unique_ptr<Texture2D> CreateTexture2D(const uint32_t width, const uint32_t height, const TextureFormat format, const uint32_t mipLevels) = 0;
      virtual std::unique_ptr<Texture2D> CreateTexture2D(const std::filesystem::path& path) = 0;

      virtual std::unique_ptr<TextureCube> CreateTextureCube(const std::filesystem::path& path) = 0;

   };


   class PKZL_API RenderCore {
   public:

      enum class API {
         Undefined,
         OpenGL,
         Vulkan
      };

      // Set the back-end API that you want to use.
      // This can only be done once, at application startup.
      // If you do not do it, then OpenGL will be chosen by default.
      // Throws a runtime_error if the specified API cannot be set,
      // in which case you could try again with a different one.
      static void SetAPI(API api);
      static API GetAPI();

      static void Init(const Window& window);
      static void DeInit();

      static void UploadImGuiFonts();

      static void SetViewport(const uint32_t x, const uint32_t y, const uint32_t width, const uint32_t height);

      static std::unique_ptr<GraphicsContext> CreateGraphicsContext(const Window& window);

      static std::unique_ptr<VertexBuffer> CreateVertexBuffer(const uint32_t size);
      static std::unique_ptr<VertexBuffer> CreateVertexBuffer(const uint32_t size, const void* data);

      static std::unique_ptr<IndexBuffer> CreateIndexBuffer(const uint32_t count, const uint32_t* indices);

      static std::unique_ptr<UniformBuffer> CreateUniformBuffer(const uint32_t size);
      static std::unique_ptr<UniformBuffer> CreateUniformBuffer(const uint32_t size, const void* data);

      static std::unique_ptr<Framebuffer> CreateFramebuffer(const FramebufferSettings& settings);

      static std::unique_ptr<Texture2D> CreateTexture2D(const uint32_t width, const uint32_t height, const TextureFormat format, const uint32_t mipLevels);
      static std::unique_ptr<Texture2D> CreateTexture2D(const std::filesystem::path& path);

      static std::unique_ptr<TextureCube> CreateTextureCube(const std::filesystem::path& path);

   private:
      inline static API s_API = API::Undefined;
      inline static std::unique_ptr<IRenderCore> s_RenderCore;

      using RENDERCORECREATEPROC = IRenderCore * (__cdecl*)(const Window*);
      inline static RENDERCORECREATEPROC CreateRenderCore;
   };

}
