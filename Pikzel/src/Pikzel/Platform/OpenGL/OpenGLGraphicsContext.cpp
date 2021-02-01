#include "OpenGLGraphicsContext.h"

#include "OpenGLBuffer.h"
#include "OpenGLFramebuffer.h"
#include "OpenGLPipeline.h"
#include "OpenGLTexture.h"

#include "Pikzel/Events/EventDispatcher.h"

#include <imgui.h>
#include <imgui_internal.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>

namespace Pikzel {

   OpenGLGraphicsContext::OpenGLGraphicsContext(const glm::vec4& clearColorValue, const GLdouble clearDepthValue)
   : m_Pipeline {nullptr}
   , m_ClearColorValue {clearColorValue}
   , m_ClearDepthValue {clearDepthValue}
   {}


   const glm::vec4 OpenGLGraphicsContext::GetClearColorValue() const {
      return m_ClearColorValue;
   }


   const GLdouble OpenGLGraphicsContext::GetClearDepthValue() const {
      return m_ClearDepthValue;
   }


   void OpenGLGraphicsContext::InitializeImGui() {
      __super::InitializeImGui();
   }


   ImGuiContext* OpenGLGraphicsContext::GetImGuiContext() {
      return ImGui::GetCurrentContext();
   }


   void OpenGLGraphicsContext::BeginImGuiFrame() {}
   void OpenGLGraphicsContext::EndImGuiFrame() {}


   void OpenGLGraphicsContext::Bind(const VertexBuffer& buffer) {
      const OpenGLVertexBuffer& glVertexBuffer = static_cast<const OpenGLVertexBuffer&>(buffer);
      glBindVertexBuffer(0, glVertexBuffer.GetRendererId(), 0, glVertexBuffer.GetLayout().GetStride());
   }


   void OpenGLGraphicsContext::Unbind(const VertexBuffer&) {
      glBindVertexBuffer(0, 0, 0, 0);
   }


   void OpenGLGraphicsContext::Bind(const IndexBuffer& buffer) {
      const OpenGLIndexBuffer& glIndexBuffer = static_cast<const OpenGLIndexBuffer&>(buffer);
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, glIndexBuffer.GetRendererId());
   }


   void OpenGLGraphicsContext::Unbind(const IndexBuffer&) {
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
   }


   void OpenGLGraphicsContext::Bind(const entt::id_type resourceId, const UniformBuffer& buffer) {
      glBindBufferBase(GL_UNIFORM_BUFFER, m_Pipeline->GetUniformBufferBinding(resourceId), static_cast<const OpenGLUniformBuffer&>(buffer).GetRendererId());
   }


   void OpenGLGraphicsContext::Unbind(const UniformBuffer&) {}


   void OpenGLGraphicsContext::Bind(const entt::id_type resourceId, const Texture& texture) {
      glBindTextureUnit(m_Pipeline->GetSamplerBinding(resourceId), static_cast<const OpenGLTexture&>(texture).GetRendererId());
   }


   void OpenGLGraphicsContext::Unbind(const Texture&) {}


   void OpenGLGraphicsContext::Bind(const Pipeline& pipeline) {
      const OpenGLPipeline& glPipeline = static_cast<const OpenGLPipeline&>(pipeline);
      glPipeline.SetGLState();
      m_Pipeline = const_cast<OpenGLPipeline*>(&glPipeline);
   }


   void OpenGLGraphicsContext::Unbind(const Pipeline& pipeline) {
      m_Pipeline = nullptr;
      glBindVertexArray(0);
      glUseProgram(0);
   }


   std::unique_ptr<Pikzel::Pipeline> OpenGLGraphicsContext::CreatePipeline(const PipelineSettings& settings) {
      return std::make_unique<OpenGLPipeline>(settings);
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

   
   void OpenGLGraphicsContext::PushConstant(const entt::id_type id, const glm::bvec3& value) {
      PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
      m_Pipeline->PushConstant(id, value);
   }


   void OpenGLGraphicsContext::PushConstant(const entt::id_type id, const glm::bvec4& value) {
      PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
      m_Pipeline->PushConstant(id, value);
   }


   void OpenGLGraphicsContext::PushConstant(const entt::id_type id, const glm::ivec2& value) {
      PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
      m_Pipeline->PushConstant(id, value);
   }


   void OpenGLGraphicsContext::PushConstant(const entt::id_type id, const glm::ivec3& value) {
      PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
      m_Pipeline->PushConstant(id, value);
   }


   void OpenGLGraphicsContext::PushConstant(const entt::id_type id, const glm::ivec4& value) {
      PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
      m_Pipeline->PushConstant(id, value);
   }


   void OpenGLGraphicsContext::PushConstant(const entt::id_type id, const glm::uvec2& value) {
      PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
      m_Pipeline->PushConstant(id, value);
   }


   void OpenGLGraphicsContext::PushConstant(const entt::id_type id, const glm::uvec3& value) {
      PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
      m_Pipeline->PushConstant(id, value);
   }


   void OpenGLGraphicsContext::PushConstant(const entt::id_type id, const glm::uvec4& value) {
      PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
      m_Pipeline->PushConstant(id, value);
   }


   void OpenGLGraphicsContext::PushConstant(const entt::id_type id, const glm::vec2& value) {
      PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
      m_Pipeline->PushConstant(id, value);
   }


   void OpenGLGraphicsContext::PushConstant(const entt::id_type id, const glm::vec3& value) {
      PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
      m_Pipeline->PushConstant(id, value);
   }


   void OpenGLGraphicsContext::PushConstant(const entt::id_type id, const glm::vec4& value) {
      PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
      m_Pipeline->PushConstant(id, value);
   }


   void OpenGLGraphicsContext::PushConstant(const entt::id_type id, const glm::dvec2& value) {
      PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
      m_Pipeline->PushConstant(id, value);
   }


   void OpenGLGraphicsContext::PushConstant(const entt::id_type id, const glm::dvec3& value) {
      PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
      m_Pipeline->PushConstant(id, value);
   }


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


   void OpenGLGraphicsContext::DrawTriangles(const VertexBuffer& vertexBuffer, const uint32_t vertexCount, const uint32_t vertexOffset/*= 0*/) {
      PKZL_PROFILE_FUNCTION();
      Bind(vertexBuffer);
      glDrawArrays(GL_TRIANGLES, vertexOffset, vertexCount);
   }


   void OpenGLGraphicsContext::DrawIndexed(const VertexBuffer& vertexBuffer, const IndexBuffer& indexBuffer, const uint32_t indexCount/*= 0*/, const uint32_t vertexOffset/*= 0*/) {
      PKZL_PROFILE_FUNCTION();
      uint32_t count = indexCount ? indexCount : indexBuffer.GetCount();
      Bind(vertexBuffer);
      Bind(indexBuffer);
      glDrawElementsBaseVertex(GL_TRIANGLES, count, GL_UNSIGNED_INT, nullptr, vertexOffset);
   }


   OpenGLWindowGC::OpenGLWindowGC(const Window& window)
   : OpenGLGraphicsContext {window.GetClearColor(), 0.0}
   , m_WindowHandle {(GLFWwindow*)window.GetNativeWindow()}
   {
      PKZL_CORE_ASSERT(m_WindowHandle, "Window handle is null!")
      EventDispatcher::Connect<WindowVSyncChangedEvent, &OpenGLWindowGC::OnWindowVSyncChanged>(*this);
      glfwSwapInterval(window.IsVSync() ? 1 : 0);
   }


   OpenGLWindowGC::~OpenGLWindowGC() {
      EventDispatcher::Disconnect<WindowVSyncChangedEvent, &OpenGLWindowGC::OnWindowVSyncChanged>(*this);
      if (ImGui::GetCurrentContext()) {
         ImGui_ImplOpenGL3_Shutdown();
         ImGui_ImplGlfw_Shutdown();
         ImGui::DestroyContext();
      }
   }


   void OpenGLWindowGC::InitializeImGui() {
      IMGUI_CHECKVERSION();
      ImGui::CreateContext();
      ImGuiIO& io = ImGui::GetIO();
      io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
      io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
      io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
      ImGui_ImplOpenGL3_Init();
      ImGui_ImplGlfw_InitForOpenGL(m_WindowHandle, true);
      __super::InitializeImGui();
   }


   void OpenGLWindowGC::BeginImGuiFrame() {
      PKZL_PROFILE_FUNCTION();
      ImGui_ImplOpenGL3_NewFrame();
      ImGui_ImplGlfw_NewFrame();
      ImGui::NewFrame();
   }


   void OpenGLWindowGC::EndImGuiFrame() {
      PKZL_PROFILE_FUNCTION();
      ImGui::Render();
      ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
      if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
         ImGui::UpdatePlatformWindows();
         ImGui::RenderPlatformWindowsDefault();
         if (glfwGetCurrentContext() != m_WindowHandle) {
            glfwMakeContextCurrent(m_WindowHandle);
         }
      }
   }


   void OpenGLWindowGC::BeginFrame(const BeginFrameOp operation) {
      PKZL_PROFILE_FUNCTION();
      {
         PKZL_PROFILE_SCOPE("glBindFramebuffer");
         glBindFramebuffer(GL_FRAMEBUFFER, 0);
      }
      {
         PKZL_PROFILE_SCOPE("glViewport");
         int width;
         int height;
         glfwGetWindowSize(m_WindowHandle, &width, &height);
         glViewport(0, 0, width, height);
      }
      {
         PKZL_PROFILE_SCOPE("glClear");
         if (operation != BeginFrameOp::ClearNone) {
            glm::vec4 clearColor = GetClearColorValue();
            switch (operation) {
               case BeginFrameOp::ClearDepth:
                  glClearDepth(GetClearDepthValue());
                  glClear(GL_DEPTH_BUFFER_BIT);
                  break;
               case BeginFrameOp::ClearColor:
                  glClearColor(clearColor.r, clearColor.g, clearColor.b, clearColor.a);
                  glClear(GL_COLOR_BUFFER_BIT);
                  break;
               case BeginFrameOp::ClearAll:
                  glClearColor(clearColor.r, clearColor.g, clearColor.b, clearColor.a);
                  glClearDepth(GetClearDepthValue());
                  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                  break;
               default:
                  PKZL_CORE_ASSERT(false, "Unknown BeginFrameOp!");
            }
         }
      }
   }


   void OpenGLWindowGC::EndFrame() {}


   void OpenGLWindowGC::OnWindowVSyncChanged(const WindowVSyncChangedEvent& event) {
      if (event.Sender == m_WindowHandle) {
         glfwSwapInterval(event.IsVSync ? 1 : 0);
      }
   }


   void OpenGLWindowGC::SwapBuffers() {
      PKZL_PROFILE_FUNCTION();
      glfwSwapBuffers(m_WindowHandle);
   }


   OpenGLFramebufferGC::OpenGLFramebufferGC(OpenGLFramebuffer* framebuffer)
   : OpenGLGraphicsContext {framebuffer->GetClearColorValue(), framebuffer->GetClearDepthValue()}
   , m_Framebuffer {framebuffer}
   {}


   void OpenGLFramebufferGC::BeginFrame(const BeginFrameOp operation) {
      PKZL_PROFILE_FUNCTION();
      {
         PKZL_PROFILE_SCOPE("glBindFramebuffer");
         glBindFramebuffer(GL_FRAMEBUFFER, m_Framebuffer->GetRendererId());
      }
      {
         PKZL_PROFILE_SCOPE("glViewport");
         glViewport(0, 0, m_Framebuffer->GetWidth(), m_Framebuffer->GetHeight());
      }
      {
         PKZL_PROFILE_SCOPE("glClear");
         if (operation != BeginFrameOp::ClearNone) {
            glm::vec4 clearColor = GetClearColorValue();
            switch (operation) {
               case BeginFrameOp::ClearDepth:
                  glClearDepth(GetClearDepthValue());
                  glClear(GL_DEPTH_BUFFER_BIT);
                  break;
               case BeginFrameOp::ClearColor:
                  glClearColor(clearColor.r, clearColor.g, clearColor.b, clearColor.a);
                  glClear(GL_COLOR_BUFFER_BIT);
                  break;
               case BeginFrameOp::ClearAll:
                  glClearColor(clearColor.r, clearColor.g, clearColor.b, clearColor.a);
                  glClearDepth(GetClearDepthValue());
                  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                  break;
               default:
                  PKZL_CORE_ASSERT(false, "Unknown BeginFrameOp!");
            }
         }
      }
   }


   void OpenGLFramebufferGC::EndFrame() {}


   void OpenGLFramebufferGC::SwapBuffers() {
      PKZL_PROFILE_FUNCTION();

      {
         PKZL_PROFILE_SCOPE("glBlitFramebuffer")
            static GLenum buffers[4] = {
               GL_COLOR_ATTACHMENT0,
               GL_COLOR_ATTACHMENT1,
               GL_COLOR_ATTACHMENT2,
               GL_COLOR_ATTACHMENT3,
         };

         if (m_Framebuffer->GetResolveRendererId()) {
            for (uint32_t i = 0; i < m_Framebuffer->GetNumColorAttachments(); ++i) {
               glBindFramebuffer(GL_READ_FRAMEBUFFER, m_Framebuffer->GetRendererId());
               glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_Framebuffer->GetResolveRendererId());
               glReadBuffer(buffers[i]);
               glDrawBuffer(buffers[i]);
               glBlitFramebuffer(
                  0, 0,
                  m_Framebuffer->GetWidth(), m_Framebuffer->GetHeight(),
                  0, 0,
                  m_Framebuffer->GetWidth(), m_Framebuffer->GetHeight(),
                  GL_COLOR_BUFFER_BIT,
                  GL_NEAREST
               );
            }

            if (m_Framebuffer->HasDepthAttachment()) {
               glBlitFramebuffer(
                  0, 0,
                  m_Framebuffer->GetWidth(), m_Framebuffer->GetHeight(),
                  0, 0,
                  m_Framebuffer->GetWidth(), m_Framebuffer->GetHeight(),
                  GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT,
                  GL_NEAREST
               );

            }
         }
      }
      {
         PKZL_PROFILE_SCOPE("glBindFramebuffer(0)");
         glBindFramebuffer(GL_FRAMEBUFFER, 0);
      }
   }
}
