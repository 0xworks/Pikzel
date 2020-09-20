#pragma once

#include "Pikzel/Renderer/Buffer.h"

namespace Pikzel {

   class OpenGLVertexBuffer : public VertexBuffer {
   public:
      OpenGLVertexBuffer(const uint32_t size);
      OpenGLVertexBuffer(const uint32_t size, const void* data);
      virtual ~OpenGLVertexBuffer();

      virtual void CopyFromHost(const uint64_t offset, const uint64_t size, const void* pData) override;

      virtual const BufferLayout& GetLayout() const override;
      virtual void SetLayout(const BufferLayout& layout) override;

      GLuint GetRendererId() const;

   private:
      GLuint m_RendererID;
      BufferLayout m_Layout;
   };


   class OpenGLIndexBuffer : public IndexBuffer {
   public:
      OpenGLIndexBuffer(const uint32_t count, const uint32_t* indices);
      virtual ~OpenGLIndexBuffer();

      virtual void CopyFromHost(const uint64_t offset, const uint64_t size, const void* pData) override;

      virtual uint32_t GetCount() const override;

      GLuint GetRendererId() const;

   private:
      GLuint m_RendererID;
      uint32_t m_Count;
   };

}