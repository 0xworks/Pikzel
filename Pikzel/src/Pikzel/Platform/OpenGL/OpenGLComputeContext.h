#pragma once

#include "Pikzel/Renderer/ComputeContext.h"

#include <glm/glm.hpp>

#include <memory>

namespace Pikzel {

   class OpenGLPipeline;

   class OpenGLComputeContext : public ComputeContext {
   public:
      OpenGLComputeContext();
      virtual ~OpenGLComputeContext() = default;

      virtual void Begin() override;
      virtual void End() override;

      virtual void Bind(const entt::id_type resourceId, const UniformBuffer& buffer) override;
      virtual void Unbind(const UniformBuffer& buffer) override;

      virtual void Bind(const entt::id_type resourceId, const Texture& texture, const uint32_t mipLevel = 0) override;
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

      virtual void Dispatch(const uint32_t x, const uint32_t y, const uint32_t z) override;

   private:
      OpenGLPipeline* m_Pipeline;
   };

}
