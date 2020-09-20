#include "RenderCore.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace Pikzel {

   // TODO: remove:
   extern std::unique_ptr<IRenderCore> CreateRenderCore(const Window& window);

   std::unique_ptr<IRenderCore> RenderCore::s_RenderCore;


   RenderCore::API RenderCore::GetAPI() {
      return s_API;
   }


   void RenderCore::Init(const Window& window) {
      // this is not a switch on s_API with call to corresponding XXXRenderCore constructor
      // because this file should not have to know about XXXRenderCore
      // All it needs to know is that _somewhere_ there is a function (Create()) that can
      // be called to to get back an object that implements IRenderCore
      s_RenderCore = CreateRenderCore(window);
   }


   void RenderCore::SetViewport(const uint32_t x, const uint32_t y, const uint32_t width, const uint32_t height) {
      s_RenderCore->SetViewport(x, y, width, height);
   }


   std::unique_ptr<GraphicsContext> RenderCore::CreateGraphicsContext(const Window& window) {
      return s_RenderCore->CreateGraphicsContext(window);
   }


   std::unique_ptr<VertexBuffer> RenderCore::CreateVertexBuffer(const uint32_t size) {
      return s_RenderCore->CreateVertexBuffer(size);
   }


   std::unique_ptr<VertexBuffer> RenderCore::CreateVertexBuffer(const uint32_t size, const void* data) {
      return s_RenderCore->CreateVertexBuffer(size, data);
   }


   std::unique_ptr<IndexBuffer> RenderCore::CreateIndexBuffer(const uint32_t count, const uint32_t* indices) {
      return s_RenderCore->CreateIndexBuffer(count, indices);
   }


   std::unique_ptr<Texture2D> RenderCore::CreateTexture2D(const uint32_t width, const uint32_t height) {
      return s_RenderCore->CreateTexture2D(width, height);
   }


   std::unique_ptr<Texture2D> RenderCore::CreateTexture2D(const std::filesystem::path& path) {
      return s_RenderCore->CreateTexture2D(path);
   }

}
