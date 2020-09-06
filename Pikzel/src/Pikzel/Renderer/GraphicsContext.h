#pragma once

#include "Buffer.h"
#include "Pipeline.h"
#include "Texture.h"

#include <memory>

namespace Pikzel {

   class GraphicsContext {
   public:
      virtual ~GraphicsContext() = default;

      virtual void BeginFrame() = 0;
      virtual void EndFrame() = 0;

      virtual void SwapBuffers() = 0;

      virtual void Bind(const VertexBuffer& buffer) = 0;
      virtual void Unbind(const VertexBuffer& buffer) = 0;

      virtual void Bind(const IndexBuffer& buffer) = 0;
      virtual void Unbind(const IndexBuffer& buffer) = 0;

      virtual void Bind(const Texture2D& texture, const uint32_t slot) = 0;
      virtual void Unbind(const Texture2D& texture) = 0;

      virtual void Bind(const Pipeline& pipeline) = 0;
      virtual void Unbind(const Pipeline& pipeline) = 0;

      virtual std::unique_ptr<Pipeline> CreatePipeline(const PipelineSettings& settings) = 0;

      virtual void DrawIndexed(VertexBuffer& vertexBuffer, IndexBuffer& indexBuffer, uint32_t indexCount = 0) = 0;

   };


   template<class T, typename... Args>
   class GCBinder {
   public:
      GCBinder(GraphicsContext& gc, const T& bindee, Args... args)
      : m_GC {gc}
      , m_Bindee {bindee}
      {
         m_GC.Bind(m_Bindee, std::forward<Args>(args)...);
      }

      ~GCBinder() {
         m_GC.Unbind(m_Bindee);
      }

   private:
      GraphicsContext& m_GC;
      const T& m_Bindee;
   };

}
