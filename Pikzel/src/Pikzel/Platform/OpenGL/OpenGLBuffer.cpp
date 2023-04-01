#include "OpenGLBuffer.h"

#include <GL/gl.h>

namespace Pikzel {

   OpenGLVertexBuffer::OpenGLVertexBuffer(const BufferLayout& layout, const uint32_t size)
   : m_Layout {layout} {
      glCreateBuffers(1, &m_RendererID);
      glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);
      glBufferData(GL_ARRAY_BUFFER, size, nullptr, GL_DYNAMIC_DRAW);
   }


   OpenGLVertexBuffer::OpenGLVertexBuffer(const BufferLayout& layout, const uint32_t size, const void* data)
   : m_Layout {layout} {
      glCreateBuffers(1, &m_RendererID);
      glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);
      glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);
   }


   OpenGLVertexBuffer::~OpenGLVertexBuffer() {
      glDeleteBuffers(1, &m_RendererID);
   }


   void OpenGLVertexBuffer::CopyFromHost(const uint64_t offset, const uint64_t size, const void* pData) {
      glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);
      glBufferSubData(GL_ARRAY_BUFFER, offset, size, pData);
   }


   const Pikzel::BufferLayout& OpenGLVertexBuffer::GetLayout() const {
      return m_Layout;
   }


   void OpenGLVertexBuffer::SetLayout(const BufferLayout& layout) {
      PKZL_CORE_ASSERT(layout.GetElements().size(), "layout is empty!");
      m_Layout = layout;
   }


   GLuint OpenGLVertexBuffer::GetRendererId() const {
      return m_RendererID;
   }


   OpenGLIndexBuffer::OpenGLIndexBuffer(const uint32_t count, const uint32_t* indices)
   : m_Count(count)
   {
      glCreateBuffers(1, &m_RendererID);

      // GL_ELEMENT_ARRAY_BUFFER is not valid without an actively bound VAO
      // Binding with GL_ARRAY_BUFFER allows the data to be loaded regardless of VAO state. 
      glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);
      glBufferData(GL_ARRAY_BUFFER, count * sizeof(uint32_t), indices, GL_STATIC_DRAW);
   }


   OpenGLIndexBuffer::~OpenGLIndexBuffer() {
      glDeleteBuffers(1, &m_RendererID);
   }


   void OpenGLIndexBuffer::CopyFromHost(const uint64_t offset, const uint64_t size, const void* pData) {
      PKZL_CORE_ASSERT(false, "Cannot SetData() on an OpenGLIndexBuffer - it is static data, set at construction!")
   }


   GLuint OpenGLIndexBuffer::GetRendererId() const {
      return m_RendererID;
   }


   uint32_t OpenGLIndexBuffer::GetCount() const {
      return m_Count;
   }


   OpenGLUniformBuffer::OpenGLUniformBuffer(const uint32_t size) {
      glCreateBuffers(1, &m_RendererID);
      glBindBuffer(GL_UNIFORM_BUFFER, m_RendererID);
      glBufferData(GL_UNIFORM_BUFFER, size, nullptr, GL_DYNAMIC_DRAW);

   }


   OpenGLUniformBuffer::OpenGLUniformBuffer(const uint32_t size, const void* data) {
      glCreateBuffers(1, &m_RendererID);
      glBindBuffer(GL_UNIFORM_BUFFER, m_RendererID);
      glBufferData(GL_UNIFORM_BUFFER, size, data, GL_STATIC_DRAW);
   }


   OpenGLUniformBuffer::~OpenGLUniformBuffer() {
      glDeleteBuffers(1, &m_RendererID);
   }


   void OpenGLUniformBuffer::CopyFromHost(const uint64_t offset, const uint64_t size, const void* pData) {
      glBindBuffer(GL_UNIFORM_BUFFER, m_RendererID);
      glBufferSubData(GL_UNIFORM_BUFFER, offset, size, pData);
   }


   GLuint OpenGLUniformBuffer::GetRendererId() const {
      return m_RendererID;
   }

}
