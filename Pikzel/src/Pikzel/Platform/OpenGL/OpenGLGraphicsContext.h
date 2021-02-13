#pragma once

#include "OpenGLFramebuffer.h"

#include "Pikzel/Core/Window.h"
#include "Pikzel/Events/WindowEvents.h"
#include "Pikzel/Renderer/GraphicsContext.h"

#include <glm/glm.hpp>

#include <memory>

struct GLFWwindow;

namespace Pikzel {

   class OpenGLPipeline;

   class OpenGLGraphicsContext : public GraphicsContext {
   using super = GraphicsContext;
   protected:
      OpenGLGraphicsContext(const glm::vec4& clearColorValue, const GLdouble clearDepthValue);
      virtual ~OpenGLGraphicsContext() = default;

   public:

      const glm::vec4 GetClearColorValue() const;
      const GLdouble GetClearDepthValue() const;

      virtual void InitializeImGui() override;
      virtual ImGuiContext* GetImGuiContext() override;
      virtual void BeginImGuiFrame() override;
      virtual void EndImGuiFrame() override;

      virtual void Bind(const VertexBuffer& buffer) override;
      virtual void Unbind(const VertexBuffer& buffer) override;

      virtual void Bind(const IndexBuffer& buffer) override;
      virtual void Unbind(const IndexBuffer& buffer) override;

      virtual void Bind(const entt::id_type resourceId, const UniformBuffer& buffer) override;
      virtual void Unbind(const UniformBuffer& buffer) override;

      virtual void Bind(const entt::id_type resourceId, const Texture& texture) override;
      virtual void Unbind(const Texture& texture) override;

      virtual void Bind(const Pipeline& pipeline) override;
      virtual void Unbind(const Pipeline& pipeline) override;

      virtual std::unique_ptr<Pipeline> CreatePipeline(const PipelineSettings& settings) override;

      virtual void PushConstant(const entt::id_type id, bool value) override;
      virtual void PushConstant(const entt::id_type id, int value) override;
      virtual void PushConstant(const entt::id_type id, uint32_t value) override;
      virtual void PushConstant(const entt::id_type id, float value) override;
      virtual void PushConstant(const entt::id_type id, double value) override;
      virtual void PushConstant(const entt::id_type id, const glm::bvec2& value) override;
      virtual void PushConstant(const entt::id_type id, const glm::bvec3& value) override;
      virtual void PushConstant(const entt::id_type id, const glm::bvec4& value) override;
      virtual void PushConstant(const entt::id_type id, const glm::ivec2& value) override;
      virtual void PushConstant(const entt::id_type id, const glm::ivec3& value) override;
      virtual void PushConstant(const entt::id_type id, const glm::ivec4& value) override;
      virtual void PushConstant(const entt::id_type id, const glm::uvec2& value) override;
      virtual void PushConstant(const entt::id_type id, const glm::uvec3& value) override;
      virtual void PushConstant(const entt::id_type id, const glm::uvec4& value) override;
      virtual void PushConstant(const entt::id_type id, const glm::vec2& value) override;
      virtual void PushConstant(const entt::id_type id, const glm::vec3& value) override;
      virtual void PushConstant(const entt::id_type id, const glm::vec4& value) override;
      virtual void PushConstant(const entt::id_type id, const glm::dvec2& value) override;
      virtual void PushConstant(const entt::id_type id, const glm::dvec3& value) override;
      virtual void PushConstant(const entt::id_type id, const glm::dvec4& value) override;
      virtual void PushConstant(const entt::id_type id, const glm::mat2& value) override;
      //virtual void PushConstant(const entt::id_type id, const glm::mat2x3& value) override;
      virtual void PushConstant(const entt::id_type id, const glm::mat2x4& value) override;
      virtual void PushConstant(const entt::id_type id, const glm::mat3x2& value) override;
      //virtual void PushConstant(const entt::id_type id, const glm::mat3& value) override;
      virtual void PushConstant(const entt::id_type id, const glm::mat3x4& value) override;
      virtual void PushConstant(const entt::id_type id, const glm::mat4x2& value) override;
      //virtual void PushConstant(const entt::id_type id, const glm::mat4x3& value) override;
      virtual void PushConstant(const entt::id_type id, const glm::mat4& value) override;
      virtual void PushConstant(const entt::id_type id, const glm::dmat2& value) override;
      //virtual void PushConstant(const entt::id_type id, const glm::dmat2x3& value) override;
      virtual void PushConstant(const entt::id_type id, const glm::dmat2x4& value) override;
      virtual void PushConstant(const entt::id_type id, const glm::dmat3x2& value) override;
      //virtual void PushConstant(const entt::id_type id, const glm::dmat3& value) override;
      virtual void PushConstant(const entt::id_type id, const glm::dmat3x4& value) override;
      virtual void PushConstant(const entt::id_type id, const glm::dmat4x2& value) override;
      //virtual void PushConstant(const entt::id_type id, const glm::dmat4x3& value) override;
      virtual void PushConstant(const entt::id_type id, const glm::dmat4& value) override;

      virtual void DrawTriangles(const VertexBuffer& vertexBuffer, const uint32_t vertexCount, const uint32_t vertexOffset = 0) override;
      virtual void DrawIndexed(const VertexBuffer& vertexBuffer, const IndexBuffer& indexBuffer, const uint32_t indexCount = 0, const uint32_t vertexOffset = 0) override;

   private:
      OpenGLPipeline* m_Pipeline;
      glm::vec4 m_ClearColorValue;
      GLdouble m_ClearDepthValue;
   };


   class OpenGLWindowGC : public OpenGLGraphicsContext {
   using super = OpenGLGraphicsContext;
   public:
      OpenGLWindowGC(const Window& window);
      ~OpenGLWindowGC();

      virtual void InitializeImGui() override;
      virtual void BeginImGuiFrame() override;
      virtual void EndImGuiFrame() override;

      virtual void BeginFrame(const BeginFrameOp operation = BeginFrameOp::ClearAll) override;
      virtual void EndFrame() override;

      virtual void SwapBuffers() override;

   private:
      void OnWindowVSyncChanged(const WindowVSyncChangedEvent& event);

   private:
      GLFWwindow* m_WindowHandle;
      bool m_InitializedImGui = false;
   };


   class OpenGLFramebufferGC : public OpenGLGraphicsContext {
   using super = OpenGLGraphicsContext;
   public:
      OpenGLFramebufferGC(OpenGLFramebuffer* framebuffer); // raw pointer is fine here.  We know the OpenGLFramebufferGC lifetime is nested inside the framebuffer's lifetime

      virtual void BeginFrame(const BeginFrameOp operation = BeginFrameOp::ClearAll) override;
      virtual void EndFrame() override;

      virtual void SwapBuffers() override;

   private:
      OpenGLFramebuffer* m_Framebuffer;
   };

}
