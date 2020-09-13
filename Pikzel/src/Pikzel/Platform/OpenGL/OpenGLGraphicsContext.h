#pragma once

#include "Pikzel/Core/Window.h"
#include "Pikzel/Renderer/GraphicsContext.h"

#include <glm/glm.hpp>

struct GLFWwindow;

namespace Pikzel {

   class OpenGLPipeline;

   class OpenGLGraphicsContext : public GraphicsContext {
   public:
      OpenGLGraphicsContext(const Window& window);

      virtual void BeginFrame() override;
      virtual void EndFrame() override;

      virtual void SwapBuffers() override;

      virtual void Bind(const VertexBuffer& buffer) override;
      virtual void Unbind(const VertexBuffer& buffer) override;

      virtual void Bind(const IndexBuffer& buffer) override;
      virtual void Unbind(const IndexBuffer& buffer) override;

      virtual void Bind(const Texture2D& texture, const uint32_t slot) override;
      virtual void Unbind(const Texture2D& texture) override;

      virtual void Bind(const Pipeline& pipeline) override;
      virtual void Unbind(const Pipeline& pipeline) override;

      virtual std::unique_ptr<Pipeline> CreatePipeline(const PipelineSettings& settings) override;

      virtual void PushConstant(const std::string& name, bool value) override;
      virtual void PushConstant(const std::string& name, int value) override;
      virtual void PushConstant(const std::string& name, uint32_t value) override;
      virtual void PushConstant(const std::string & name, float value) override;
      virtual void PushConstant(const std::string& name, double value) override;
      virtual void PushConstant(const std::string& name, const glm::bvec2& value) override;
      virtual void PushConstant(const std::string& name, const glm::bvec3& value) override;
      virtual void PushConstant(const std::string& name, const glm::bvec4& value) override;
      virtual void PushConstant(const std::string& name, const glm::ivec2& value) override;
      virtual void PushConstant(const std::string& name, const glm::ivec3& value) override;
      virtual void PushConstant(const std::string& name, const glm::ivec4& value) override;
      virtual void PushConstant(const std::string& name, const glm::uvec2& value) override;
      virtual void PushConstant(const std::string& name, const glm::uvec3& value) override;
      virtual void PushConstant(const std::string& name, const glm::uvec4& value) override;
      virtual void PushConstant(const std::string& name, const glm::vec2& value) override;
      virtual void PushConstant(const std::string& name, const glm::vec3& value) override;
      virtual void PushConstant(const std::string& name, const glm::vec4& value) override;
      virtual void PushConstant(const std::string& name, const glm::dvec2& value) override;
      virtual void PushConstant(const std::string& name, const glm::dvec3& value) override;
      virtual void PushConstant(const std::string& name, const glm::dvec4& value) override;
      virtual void PushConstant(const std::string& name, const glm::mat2& value) override;
      virtual void PushConstant(const std::string& name, const glm::mat2x3& value) override;
      virtual void PushConstant(const std::string& name, const glm::mat2x4& value) override;
      virtual void PushConstant(const std::string& name, const glm::mat3x2& value) override;
      virtual void PushConstant(const std::string& name, const glm::mat3& value) override;
      virtual void PushConstant(const std::string& name, const glm::mat3x4& value) override;
      virtual void PushConstant(const std::string& name, const glm::mat4x2& value) override;
      virtual void PushConstant(const std::string& name, const glm::mat4x3& value) override;
      virtual void PushConstant(const std::string& name, const glm::mat4& value) override;
      virtual void PushConstant(const std::string& name, const glm::dmat2& value) override;
      virtual void PushConstant(const std::string& name, const glm::dmat2x3& value) override;
      virtual void PushConstant(const std::string& name, const glm::dmat2x4& value) override;
      virtual void PushConstant(const std::string& name, const glm::dmat3x2& value) override;
      virtual void PushConstant(const std::string& name, const glm::dmat3& value) override;
      virtual void PushConstant(const std::string& name, const glm::dmat3x4& value) override;
      virtual void PushConstant(const std::string& name, const glm::dmat4x2& value) override;
      virtual void PushConstant(const std::string& name, const glm::dmat4x3& value) override;
      virtual void PushConstant(const std::string& name, const glm::dmat4& value) override;

      virtual void DrawIndexed(const VertexBuffer& vertexBuffer, const IndexBuffer& indexBuffer, uint32_t indexCount = 0) override;

   private:
      GLFWwindow* m_WindowHandle;
      OpenGLPipeline* m_Pipeline;
      glm::vec4 m_ClearColor;
   };

}