#pragma once

#include "Buffer.h"
#include "GraphicsContext.h"
#include "Shader.h"
#include "VertexArray.h"
#include "Pikzel/Core/Window.h"

#include <glm/glm.hpp>
#include <memory>
#include <vector>

namespace Pikzel {

   struct IRenderCore {
      virtual ~IRenderCore() = default;

      virtual void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) = 0;

      virtual void SetClearColor(const glm::vec4& color) = 0;
      virtual void Clear() = 0;

      virtual std::unique_ptr<GraphicsContext> CreateGraphicsContext(Window& window) = 0;

      virtual std::unique_ptr<VertexBuffer> CreateVertexBuffer(uint32_t size) = 0;
      virtual std::unique_ptr<VertexBuffer> CreateVertexBuffer(float* vertices, uint32_t size) = 0;

      virtual std::unique_ptr<IndexBuffer> CreateIndexBuffer(uint32_t* indices, uint32_t count) = 0;

      // TODO: this needs to be hidden away somehow, as rendering APIs other than OpenGL do not have "vertex array objects"
      virtual std::unique_ptr<VertexArray> CreateVertexArray() = 0;

      // TODO: obvs there are other sorts of shader.. so this function signature will need changing...
      virtual std::unique_ptr<Shader> CreateShader(const std::vector<char>& vertexSrc, const std::vector<char>& fragmentSrc) = 0;

      virtual void DrawIndexed(VertexArray& vertexArray, uint32_t indexCount = 0) = 0;

   };


   class RenderCore {
   public:

      enum class API {
         None,
         OpenGL
      };

      static void SetAPI(API api);
      static API GetAPI();

      static void Init();

      static void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height);

      static void SetClearColor(const glm::vec4& color);
      static void Clear();

      static std::unique_ptr<GraphicsContext> CreateGraphicsContext(Window& window);

      static std::unique_ptr<VertexBuffer> CreateVertexBuffer(uint32_t size);
      static std::unique_ptr<VertexBuffer> CreateVertexBuffer(float* vertices, uint32_t size);

      static std::unique_ptr<IndexBuffer> CreateIndexBuffer(uint32_t* indices, uint32_t count);

      static std::unique_ptr<VertexArray> CreateVertexArray();

      static std::unique_ptr<Shader> CreateShader(const std::vector<char>& vertexSrc, const std::vector<char>& fragmentSrc);

      static void DrawIndexed(VertexArray& vertexArray, uint32_t indexCount = 0);

   private:
      static API s_API;
      static std::unique_ptr<IRenderCore> s_RenderCore;
   };

}
