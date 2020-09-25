#include "OpenGLGraphicsContext.h"

#include "OpenGLBuffer.h"
#include "OpenGLPipeline.h"
#include "OpenGLTexture.h"

namespace Pikzel {

   OpenGLGraphicsContext::OpenGLGraphicsContext(const Window& window)
   : m_WindowHandle {(GLFWwindow*)window.GetNativeWindow()}
   , m_Pipeline {nullptr}
   , m_ClearColor {window.GetClearColor()}
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


   void OpenGLGraphicsContext::Bind(const Texture2D& texture, const entt::id_type id) {
      glBindTextureUnit(m_Pipeline->GetSamplerBinding(id), static_cast<const OpenGLTexture2D&>(texture).GetRendererID());
   }


   void OpenGLGraphicsContext::Unbind(const Texture2D&) {
      glBindTextureUnit(0, 0);
   }


   void OpenGLGraphicsContext::Bind(const Pipeline& pipeline) {
      const OpenGLPipeline& glPipeline = reinterpret_cast<const OpenGLPipeline&>(pipeline);
      glUseProgram(glPipeline.GetRendererId());
      glBindVertexArray(glPipeline.GetVAORendererId());
      m_Pipeline = const_cast<OpenGLPipeline*>(&glPipeline);
   }


   void OpenGLGraphicsContext::Unbind(const Pipeline& pipeline) {
      m_Pipeline = nullptr;
      glBindVertexArray(0);
      glUseProgram(0);
   }


   std::unique_ptr<Pikzel::Pipeline> OpenGLGraphicsContext::CreatePipeline(const PipelineSettings& settings) {
      return std::make_unique<OpenGLPipeline>(*this, settings);
   }


   void OpenGLGraphicsContext::PushConstant(const entt::id_type id, bool value) {
      PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
      m_Pipeline->PushConstant(id, value);
   }


   void OpenGLGraphicsContext::PushConstant(const entt::id_type id, int value) {
      PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
      m_Pipeline->PushConstant(id, value);
   }


   void OpenGLGraphicsContext::PushConstant(const entt::id_type id, uint32_t value) {
      PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
      m_Pipeline->PushConstant(id, value);
   }


   void OpenGLGraphicsContext::PushConstant(const entt::id_type id, float value) {
      PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
      m_Pipeline->PushConstant(id, value);
   }


   void OpenGLGraphicsContext::PushConstant(const entt::id_type id, double value) {
      PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
      m_Pipeline->PushConstant(id, value);
   }


   void OpenGLGraphicsContext::PushConstant(const entt::id_type id, const glm::bvec2& value) {
      PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
      m_Pipeline->PushConstant(id, value);
   }

   
   //void OpenGLGraphicsContext::PushConstant(const entt::id_type id, const glm::bvec3& value) {
   //   PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
   //   m_Pipeline->PushConstant(id, value);
   //}


   void OpenGLGraphicsContext::PushConstant(const entt::id_type id, const glm::bvec4& value) {
      PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
      m_Pipeline->PushConstant(id, value);
   }


   void OpenGLGraphicsContext::PushConstant(const entt::id_type id, const glm::ivec2& value) {
      PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
      m_Pipeline->PushConstant(id, value);
   }


   //void OpenGLGraphicsContext::PushConstant(const entt::id_type id, const glm::ivec3& value) {
   //   PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
   //   m_Pipeline->PushConstant(id, value);
   //}


   void OpenGLGraphicsContext::PushConstant(const entt::id_type id, const glm::ivec4& value) {
      PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
      m_Pipeline->PushConstant(id, value);
   }


   void OpenGLGraphicsContext::PushConstant(const entt::id_type id, const glm::uvec2& value) {
      PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
      m_Pipeline->PushConstant(id, value);
   }


   //void OpenGLGraphicsContext::PushConstant(const entt::id_type id, const glm::uvec3& value) {
   //   PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
   //   m_Pipeline->PushConstant(id, value);
   //}


   void OpenGLGraphicsContext::PushConstant(const entt::id_type id, const glm::uvec4& value) {
      PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
      m_Pipeline->PushConstant(id, value);
   }


   void OpenGLGraphicsContext::PushConstant(const entt::id_type id, const glm::vec2& value) {
      PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
      m_Pipeline->PushConstant(id, value);
   }


   //void OpenGLGraphicsContext::PushConstant(const entt::id_type id, const glm::vec3& value) {
   //   PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
   //   m_Pipeline->PushConstant(id, value);
   //}


   void OpenGLGraphicsContext::PushConstant(const entt::id_type id, const glm::vec4& value) {
      PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
      m_Pipeline->PushConstant(id, value);
   }


   void OpenGLGraphicsContext::PushConstant(const entt::id_type id, const glm::dvec2& value) {
      PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
      m_Pipeline->PushConstant(id, value);
   }


   //void OpenGLGraphicsContext::PushConstant(const entt::id_type id, const glm::dvec3& value) {
   //   PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
   //   m_Pipeline->PushConstant(id, value);
   //}


   void OpenGLGraphicsContext::PushConstant(const entt::id_type id, const glm::dvec4& value) {
      PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
      m_Pipeline->PushConstant(id, value);
   }


   void OpenGLGraphicsContext::PushConstant(const entt::id_type id, const glm::mat2& value) {
      PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
      m_Pipeline->PushConstant(id, value);
   }


   //void OpenGLGraphicsContext::PushConstant(const entt::id_type id, const glm::mat2x3& value) {
   //   PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
   //   m_Pipeline->PushConstant(id, value);
   //}


   void OpenGLGraphicsContext::PushConstant(const entt::id_type id, const glm::mat2x4& value) {
      PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
      m_Pipeline->PushConstant(id, value);
   }


   void OpenGLGraphicsContext::PushConstant(const entt::id_type id, const glm::mat3x2& value) {
      PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
      m_Pipeline->PushConstant(id, value);
   }


   //void OpenGLGraphicsContext::PushConstant(const entt::id_type id, const glm::mat3& value) {
   //   PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
   //   m_Pipeline->PushConstant(id, value);
   //}


   void OpenGLGraphicsContext::PushConstant(const entt::id_type id, const glm::mat3x4& value) {
      PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
      m_Pipeline->PushConstant(id, value);
   }


   void OpenGLGraphicsContext::PushConstant(const entt::id_type id, const glm::mat4x2& value) {
      PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
      m_Pipeline->PushConstant(id, value);
   }


   //void OpenGLGraphicsContext::PushConstant(const entt::id_type id, const glm::mat4x3& value) {
   //   PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
   //   m_Pipeline->PushConstant(id, value);
   //}


   void OpenGLGraphicsContext::PushConstant(const entt::id_type id, const glm::mat4& value) {
      PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
      m_Pipeline->PushConstant(id, value);
   }


   void OpenGLGraphicsContext::PushConstant(const entt::id_type id, const glm::dmat2& value) {
      PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
      m_Pipeline->PushConstant(id, value);
   }


   //void OpenGLGraphicsContext::PushConstant(const entt::id_type id, const glm::dmat2x3& value) {
   //   PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
   //   m_Pipeline->PushConstant(id, value);
   //}


   void OpenGLGraphicsContext::PushConstant(const entt::id_type id, const glm::dmat2x4& value) {
      PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
      m_Pipeline->PushConstant(id, value);
   }


   void OpenGLGraphicsContext::PushConstant(const entt::id_type id, const glm::dmat3x2& value) {
      PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
      m_Pipeline->PushConstant(id, value);
   }


   //void OpenGLGraphicsContext::PushConstant(const entt::id_type id, const glm::dmat3& value) {
   //   PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
   //   m_Pipeline->PushConstant(id, value);
   //}


   void OpenGLGraphicsContext::PushConstant(const entt::id_type id, const glm::dmat3x4& value) {
      PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
      m_Pipeline->PushConstant(id, value);
   }


   void OpenGLGraphicsContext::PushConstant(const entt::id_type id, const glm::dmat4x2& value) {
      PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
      m_Pipeline->PushConstant(id, value);
   }


   //void OpenGLGraphicsContext::PushConstant(const entt::id_type id, const glm::dmat4x3& value) {
   //   PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
   //   m_Pipeline->PushConstant(id, value);
   //}


   void OpenGLGraphicsContext::PushConstant(const entt::id_type id, const glm::dmat4& value) {
      PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
      m_Pipeline->PushConstant(id, value);
   }


   void OpenGLGraphicsContext::DrawIndexed(const VertexBuffer& vertexBuffer, const IndexBuffer& indexBuffer, uint32_t indexCount /*= 0*/) {
      uint32_t count = indexCount ? indexCount : indexBuffer.GetCount();
      GCBinder bindVB {*this, vertexBuffer};
      GCBinder bindIB {*this, indexBuffer};
      glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, nullptr);
   }

}
