#include "glpch.h"
#include "OpenGLGraphicsContext.h"

#include "OpenGLBuffer.h"
#include "OpenGLPipeline.h"

namespace Pikzel {

   OpenGLGraphicsContext::OpenGLGraphicsContext(const Window& window)
   : m_WindowHandle((GLFWwindow*)window.GetNativeWindow())
   , m_ClearColor(window.GetClearColor())
   {
      PKZL_CORE_ASSERT(m_WindowHandle, "Window handle is null!")
   }


   void OpenGLGraphicsContext::BeginFrame() {
      glfwMakeContextCurrent(m_WindowHandle);
      glClearColor(m_ClearColor.r, m_ClearColor.g, m_ClearColor.b, m_ClearColor.a);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
   }


   void OpenGLGraphicsContext::EndFrame() {
      ;
   }


   void OpenGLGraphicsContext::SwapBuffers() {
      glfwSwapBuffers(m_WindowHandle);
   }


   void OpenGLGraphicsContext::Bind(const VertexBuffer& buffer) {
      const OpenGLVertexBuffer& glVertexBuffer = reinterpret_cast<const OpenGLVertexBuffer&>(buffer);
      glBindBuffer(GL_ARRAY_BUFFER, glVertexBuffer.GetRendererId());
   }


   void OpenGLGraphicsContext::Unbind(const VertexBuffer&) {
      glBindBuffer(GL_ARRAY_BUFFER, 0);
   }


   void OpenGLGraphicsContext::Bind(const IndexBuffer& buffer) {
      const OpenGLIndexBuffer& glIndexBuffer = reinterpret_cast<const OpenGLIndexBuffer&>(buffer);
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, glIndexBuffer.GetRendererId());
   }


   void OpenGLGraphicsContext::Unbind(const IndexBuffer&) {
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
   }


   void OpenGLGraphicsContext::Bind(const Texture2D& texture, const uint32_t slot) {
      glBindTextureUnit(slot, texture.GetRendererID());
   }


   void OpenGLGraphicsContext::Unbind(const Texture2D&) {
      glBindTextureUnit(0, 0);
   }


   void OpenGLGraphicsContext::Bind(const Pipeline& pipeline) {
      const OpenGLPipeline& glPipeline = reinterpret_cast<const OpenGLPipeline&>(pipeline);
      glUseProgram(glPipeline.GetRendererId());
      glBindVertexArray(glPipeline.GetVAORendererId());
   }


   void OpenGLGraphicsContext::Unbind(const Pipeline& pipeline) {
      glBindVertexArray(0);
      glUseProgram(0);
   }


   std::unique_ptr<Pikzel::Pipeline> OpenGLGraphicsContext::CreatePipeline(const PipelineSettings& settings) {
      return std::make_unique<OpenGLPipeline>(*this, settings);
   }


   void OpenGLGraphicsContext::DrawIndexed(VertexBuffer& vertexBuffer, IndexBuffer& indexBuffer, uint32_t indexCount /*= 0*/) {
      uint32_t count = indexCount ? indexCount : indexBuffer.GetCount();
      GCBinder bindVB {*this, vertexBuffer};
      GCBinder bindIB {*this, indexBuffer};
      glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, nullptr);
   }

}
