#include "pch.h"
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


   void RenderCore::SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) {
      s_RenderCore->SetViewport(x, y, width, height);
   }


   void RenderCore::SetClearColor(const glm::vec4& color) {
      s_RenderCore->SetClearColor(color);
   }


   void RenderCore::Clear() {
      s_RenderCore->Clear();
   }


   std::unique_ptr<GraphicsContext> RenderCore::CreateGraphicsContext(const Window& window) {
      return s_RenderCore->CreateGraphicsContext(window);
   }


   std::unique_ptr<VertexBuffer> RenderCore::CreateVertexBuffer(uint32_t size) {
      return s_RenderCore->CreateVertexBuffer(size);
   }


   std::unique_ptr<VertexBuffer> RenderCore::CreateVertexBuffer(float* vertices, uint32_t size) {
      return s_RenderCore->CreateVertexBuffer(vertices, size);
   }


   std::unique_ptr<IndexBuffer> RenderCore::CreateIndexBuffer(uint32_t* indices, uint32_t count) {
      return s_RenderCore->CreateIndexBuffer(indices, count);
   }


   std::unique_ptr<Texture2D> RenderCore::CreateTexture2D(uint32_t width, uint32_t height) {
      return s_RenderCore->CreateTexture2D(width, height);
   }


   std::unique_ptr<Texture2D> RenderCore::CreateTexture2D(const std::filesystem::path& path) {
      return s_RenderCore->CreateTexture2D(path);
   }


   std::unique_ptr<Pipeline> RenderCore::CreatePipeline(const Window& window, const PipelineSettings& settings) {
      return s_RenderCore->CreatePipeline(window, settings);
   }


   void RenderCore::DrawIndexed(VertexBuffer& vertexBuffer, IndexBuffer& indexBuffer, uint32_t indexCount /*= 0*/) {
      return s_RenderCore->DrawIndexed(vertexBuffer, indexBuffer, indexCount);
   }

}
