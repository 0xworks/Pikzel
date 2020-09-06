#pragma once

#include "Pikzel/Core/Window.h"
#include "Pikzel/Renderer/GraphicsContext.h"

#include <glm/glm.hpp>

struct GLFWwindow;

namespace Pikzel {

   class OpenGLGraphicsContext : public GraphicsContext {
   public:
      OpenGLGraphicsContext(const Window& window);

      virtual void BeginFrame() override;
      virtual void EndFrame() override;

      virtual void SwapBuffers() override;

      void Bind(const VertexBuffer& buffer) override;
      void Unbind(const VertexBuffer& buffer) override;

      void Bind(const IndexBuffer& buffer) override;
      void Unbind(const IndexBuffer& buffer) override;

      virtual void Bind(const Texture2D& texture, const uint32_t slot) override;
      virtual void Unbind(const Texture2D& texture) override;

      void Bind(const Pipeline& pipeline) override;
      void Unbind(const Pipeline& pipeline) override;

      virtual std::unique_ptr<Pipeline> CreatePipeline(const PipelineSettings& settings) override;

      virtual void DrawIndexed(VertexBuffer& vertexBuffer, IndexBuffer& indexBuffer, uint32_t indexCount = 0) override;



   private:
      GLFWwindow* m_WindowHandle;
      glm::vec4 m_ClearColor;
   };

}