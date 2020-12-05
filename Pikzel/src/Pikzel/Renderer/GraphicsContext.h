#pragma once

#include "Buffer.h"
#include "Pipeline.h"
#include "Texture.h"

#include <imgui.h>
#include <memory>

namespace Pikzel {

   class PKZL_API GraphicsContext {
   public:
      virtual ~GraphicsContext() = default;

      virtual void BeginFrame() = 0;
      virtual void EndFrame() = 0;

      // These don't belong here - what if client doesn't want ImGui baggage?  TODO: move somewhere else.
      virtual void InitializeImGui() {
         // This is a bit nasty.
         // The thing is, ImGui is initialised by the back-end renderer's graphics context.  That's in a different shared library, which effectively has its own copy of ImGui.
         // We need to make sure Pikzel's copy of ImGui shares the same context
         ImGui::SetCurrentContext(GetImGuiContext());
      }
      virtual ImGuiContext* GetImGuiContext() = 0;
      virtual void BeginImGuiFrame() = 0;
      virtual void EndImGuiFrame() = 0;

      virtual void SwapBuffers() = 0;

      virtual void Bind(const VertexBuffer& buffer) = 0;
      virtual void Unbind(const VertexBuffer& buffer) = 0;

      virtual void Bind(const IndexBuffer& buffer) = 0;
      virtual void Unbind(const IndexBuffer& buffer) = 0;

      virtual void Bind(const UniformBuffer& buffer, const entt::id_type resourceId) = 0;
      virtual void Unbind(const UniformBuffer& buffer) = 0;

      virtual void Bind(const Texture2D& texture, const entt::id_type resourceId) = 0;
      virtual void Unbind(const Texture2D& texture) = 0;

      virtual void Bind(const TextureCube& texture, const entt::id_type resourceId) = 0;
      virtual void Unbind(const TextureCube& texture) = 0;

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

      // Draw contents of vertex buffer, assuming vertices are in groups of 3, representing triangles.
      // You must specify the number of vertices (a multiple of 3).  Drawing starts from [vertexOffset]th element of the
      // vertex buffer (default 0)
      virtual void DrawTriangles(const VertexBuffer& vertexBuffer, const uint32_t vertexCount, const uint32_t vertexOffset = 0) = 0;

      // Draw contents of vertex buffer, as triangles indexed by index buffer.
      // The number of vertices drawn is determined by the number of indices in the index buffer, unless you override the indexCount parameter.
      // Drawing starts from [vertexOffset]th element of the vertex buffer (default 0)
      virtual void DrawIndexed(const VertexBuffer& vertexBuffer, const IndexBuffer& indexBuffer, const uint32_t indexCount = 0, const uint32_t vertexOffset = 0) = 0;

   };

}
