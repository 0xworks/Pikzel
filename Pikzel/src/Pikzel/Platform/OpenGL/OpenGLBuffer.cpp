#include "glpch.h"
#include "OpenGLBuffer.h"

namespace Pikzel {

   OpenGLVertexBuffer::OpenGLVertexBuffer(uint32_t size) {
      PKZL_PROFILE_FUNCTION();
      glCreateBuffers(1, &m_RendererID);
      glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);
      glBufferData(GL_ARRAY_BUFFER, size, nullptr, GL_DYNAMIC_DRAW);
   }


   OpenGLVertexBuffer::OpenGLVertexBuffer(float* vertices, uint32_t size) {
      PKZL_PROFILE_FUNCTION();
      glCreateBuffers(1, &m_RendererID);
      glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);
      glBufferData(GL_ARRAY_BUFFER, size, vertices, GL_STATIC_DRAW);
   }


   OpenGLVertexBuffer::~OpenGLVertexBuffer() {
      PKZL_PROFILE_FUNCTION();
      glDeleteBuffers(1, &m_RendererID);
   }


   void OpenGLVertexBuffer::Bind() const {
      PKZL_PROFILE_FUNCTION();
      glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);
   }

   void OpenGLVertexBuffer::Unbind() const {
      PKZL_PROFILE_FUNCTION();
      glBindBuffer(GL_ARRAY_BUFFER, 0);
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


   OpenGLIndexBuffer::OpenGLIndexBuffer(uint32_t* indices, uint32_t count)
   : m_Count(count)
   {
      PKZL_PROFILE_FUNCTION();
      glCreateBuffers(1, &m_RendererID);

      // GL_ELEMENT_ARRAY_BUFFER is not valid without an actively bound VAO
      // Binding with GL_ARRAY_BUFFER allows the data to be loaded regardless of VAO state. 
      glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);
      glBufferData(GL_ARRAY_BUFFER, count * sizeof(uint32_t), indices, GL_STATIC_DRAW);
   }


   OpenGLIndexBuffer::~OpenGLIndexBuffer() {
      PKZL_PROFILE_FUNCTION();
      glDeleteBuffers(1, &m_RendererID);
   }


   void OpenGLIndexBuffer::Bind() const {
      PKZL_PROFILE_FUNCTION();
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_RendererID);
   }


   void OpenGLIndexBuffer::Unbind() const {
      PKZL_PROFILE_FUNCTION();
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
   }


   void OpenGLIndexBuffer::CopyFromHost(const uint64_t offset, const uint64_t size, const void* pData) {
      PKZL_CORE_ASSERT(false, "Cannot SetData() on an OpenGLIndexBuffer - it is static data, set at construction!")
   }


   uint32_t OpenGLIndexBuffer::GetCount() const {
      return m_Count;
   }

}
