#pragma once

#include "Buffer.h"
#include "Pipeline.h"
#include "Texture.h"

#include <memory>

namespace Pikzel {

   class PKZL_API ComputeContext {
   public:
      virtual ~ComputeContext() = default;

      virtual void Begin() = 0;
      virtual void End() = 0;

      virtual void Bind(const UniformBuffer& buffer, const entt::id_type resourceId) = 0;
      virtual void Unbind(const UniformBuffer& buffer) = 0;

      virtual void Bind(const Texture& texture, const entt::id_type resourceId) = 0;
      virtual void Unbind(const Texture& texture) = 0;

      virtual void Bind(const Pipeline& pipeline) = 0;
      virtual void Unbind(const Pipeline& pipeline) = 0;

      virtual std::unique_ptr<Pipeline> CreatePipeline(const PipelineSettings& settings) = 0;

      // Methods dealing with arrays of 3-element vectors are not implemented.
      // The reason for this is to avoid some alignment headaches.
      // For example, in glsl there may be some padding between each column of a matrix
      // that's not how it looks on the c++ side.
      // Easiest way to avoid this headache is to just avoid having column-major matrices with 3 rows
      // in the API to begin with.  e.g. Use mat4 instead.
      virtual void PushConstant(const entt::id_type id, bool value) = 0;
      virtual void PushConstant(const entt::id_type id, int value) = 0;
      virtual void PushConstant(const entt::id_type id, uint32_t value) = 0;
      virtual void PushConstant(const entt::id_type id, float value) = 0;
      virtual void PushConstant(const entt::id_type id, double value) = 0;
      virtual void PushConstant(const entt::id_type id, const glm::bvec2& value) = 0;
      virtual void PushConstant(const entt::id_type id, const glm::bvec3& value) = 0;
      virtual void PushConstant(const entt::id_type id, const glm::bvec4& value) = 0;
      virtual void PushConstant(const entt::id_type id, const glm::ivec2& value) = 0;
      virtual void PushConstant(const entt::id_type id, const glm::ivec3& value) = 0;
      virtual void PushConstant(const entt::id_type id, const glm::ivec4& value) = 0;
      virtual void PushConstant(const entt::id_type id, const glm::uvec2& value) = 0;
      virtual void PushConstant(const entt::id_type id, const glm::uvec3& value) = 0;
      virtual void PushConstant(const entt::id_type id, const glm::uvec4& value) = 0;
      virtual void PushConstant(const entt::id_type id, const glm::vec2& value) = 0;
      virtual void PushConstant(const entt::id_type id, const glm::vec3& value) = 0;
      virtual void PushConstant(const entt::id_type id, const glm::vec4& value) = 0;
      virtual void PushConstant(const entt::id_type id, const glm::dvec2& value) = 0;
      virtual void PushConstant(const entt::id_type id, const glm::dvec3& value) = 0;
      virtual void PushConstant(const entt::id_type id, const glm::dvec4& value) = 0;
      virtual void PushConstant(const entt::id_type id, const glm::mat2& value) = 0;
      //virtual void PushConstant(const entt::id_type id, const glm::mat2x3& value) = 0;
      virtual void PushConstant(const entt::id_type id, const glm::mat2x4& value) = 0;
      virtual void PushConstant(const entt::id_type id, const glm::mat3x2& value) = 0;
      //virtual void PushConstant(const entt::id_type id, const glm::mat3& value) = 0;
      virtual void PushConstant(const entt::id_type id, const glm::mat3x4& value) = 0;
      virtual void PushConstant(const entt::id_type id, const glm::mat4x2& value) = 0;
      //virtual void PushConstant(const entt::id_type id, const glm::mat4x3& value) = 0;
      virtual void PushConstant(const entt::id_type id, const glm::mat4& value) = 0;
      virtual void PushConstant(const entt::id_type id, const glm::dmat2& value) = 0;
      //virtual void PushConstant(const entt::id_type id, const glm::dmat2x3& value) = 0;
      virtual void PushConstant(const entt::id_type id, const glm::dmat2x4& value) = 0;
      virtual void PushConstant(const entt::id_type id, const glm::dmat3x2& value) = 0;
      //virtual void PushConstant(const entt::id_type id, const glm::dmat3& value) = 0;
      virtual void PushConstant(const entt::id_type id, const glm::dmat3x4& value) = 0;
      virtual void PushConstant(const entt::id_type id, const glm::dmat4x2& value) = 0;
      //virtual void PushConstant(const entt::id_type id, const glm::dmat4x3& value) = 0;
      virtual void PushConstant(const entt::id_type id, const glm::dmat4& value) = 0;

      virtual void Dispatch(const uint32_t x, const uint32_t y, const uint32_t z) = 0;

   };

}
