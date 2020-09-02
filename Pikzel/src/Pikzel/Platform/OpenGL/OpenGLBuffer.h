#pragma once

#include "Pikzel/Renderer/Buffer.h"

namespace Pikzel {

   class OpenGLVertexBuffer : public VertexBuffer {
   public:
      OpenGLVertexBuffer(uint32_t size);
      OpenGLVertexBuffer(float* vertices, uint32_t size);
      virtual ~OpenGLVertexBuffer();

      virtual void Bind() const override;
      virtual void Unbind() const override;

      virtual void CopyFromHost(const uint64_t offset, const uint64_t size, const void* pData) override;

      virtual const BufferLayout& GetLayout() const override;
      virtual void SetLayout(const BufferLayout& layout) override;

   private:
      uint32_t m_RendererID;
      BufferLayout m_Layout;
   };


   class OpenGLIndexBuffer : public IndexBuffer {
   public:
      OpenGLIndexBuffer(uint32_t* indices, uint32_t count);
      virtual ~OpenGLIndexBuffer();

      virtual void Bind() const;
      virtual void Unbind() const;

      virtual void CopyFromHost(const uint64_t offset, const uint64_t size, const void* pData) override;

      virtual uint32_t GetCount() const override;

   private:
      uint32_t m_RendererID;
      uint32_t m_Count;
   };

}