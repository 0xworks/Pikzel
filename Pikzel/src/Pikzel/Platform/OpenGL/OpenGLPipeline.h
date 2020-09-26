#pragma once

#include "Pikzel/Renderer/GraphicsContext.h"
#include "Pikzel/Renderer/Pipeline.h"
#include "Pikzel/Renderer/ShaderUtil.h"

#include <glm/glm.hpp>

#include <unordered_map>
#include <vector>

namespace Pikzel {

   struct OpenGLUniform {
      std::string Name;
      DataType Type = DataType::None;
      int Location = 0;
      uint32_t Offset = 0;
      uint32_t Size = 0;
   };


   struct OpenGLUniformBuffer {
      std::string Name;
      uint32_t Binding = 0;
      uint32_t Size = 0;
      uint32_t RendererId = 0;
      std::vector<OpenGLUniform> Uniforms;
   };


   struct OpenGLResourceDeclaration {
      std::string Name;
      uint32_t Binding = 0;
      uint32_t Count = 0;
   };


   class OpenGLPipeline : public Pipeline {
   public:
      OpenGLPipeline(GraphicsContext& gc, const PipelineSettings& settings);
      virtual ~OpenGLPipeline();

      GLuint GetRendererId() const;
      GLuint GetVAORendererId() const;

      void PushConstant(const entt::id_type name, bool value);
      void PushConstant(const entt::id_type name, int value);
      void PushConstant(const entt::id_type name, uint32_t value);
      void PushConstant(const entt::id_type name, float value);
      void PushConstant(const entt::id_type name, double value);
      void PushConstant(const entt::id_type name, const glm::bvec2& value);
      void PushConstant(const entt::id_type name, const glm::bvec3& value);
      void PushConstant(const entt::id_type name, const glm::bvec4& value);
      void PushConstant(const entt::id_type name, const glm::ivec2& value);
      void PushConstant(const entt::id_type name, const glm::ivec3& value);
      void PushConstant(const entt::id_type name, const glm::ivec4& value);
      void PushConstant(const entt::id_type name, const glm::uvec2& value);
      void PushConstant(const entt::id_type name, const glm::uvec3& value);
      void PushConstant(const entt::id_type name, const glm::uvec4& value);
      void PushConstant(const entt::id_type name, const glm::vec2& value);
      void PushConstant(const entt::id_type name, const glm::vec3& value);
      void PushConstant(const entt::id_type name, const glm::vec4& value);
      void PushConstant(const entt::id_type name, const glm::dvec2& value);
      void PushConstant(const entt::id_type name, const glm::dvec3& value);
      void PushConstant(const entt::id_type name, const glm::dvec4& value);
      void PushConstant(const entt::id_type name, const glm::mat2& value);
      //void PushConstant(const entt::id_type name, const glm::mat2x3& value);
      void PushConstant(const entt::id_type name, const glm::mat2x4& value);
      void PushConstant(const entt::id_type name, const glm::mat3x2& value);
      //void PushConstant(const entt::id_type name, const glm::mat3& value);
      void PushConstant(const entt::id_type name, const glm::mat3x4& value);
      void PushConstant(const entt::id_type name, const glm::mat4x2& value);
      //void PushConstant(const entt::id_type name, const glm::mat4x3& value);
      void PushConstant(const entt::id_type name, const glm::mat4& value);
      void PushConstant(const entt::id_type name, const glm::dmat2& value);
      //void PushConstant(const entt::id_type name, const glm::dmat2x3& value);
      void PushConstant(const entt::id_type name, const glm::dmat2x4& value);
      void PushConstant(const entt::id_type name, const glm::dmat3x2& value);
      //void PushConstant(const entt::id_type name, const glm::dmat3& value);
      void PushConstant(const entt::id_type name, const glm::dmat3x4& value);
      void PushConstant(const entt::id_type name, const glm::dmat4x2& value);
      //void PushConstant(const entt::id_type name, const glm::dmat4x3& value);
      void PushConstant(const entt::id_type name, const glm::dmat4& value);

      GLuint GetSamplerBinding(const entt::id_type resourceId) const;
      GLuint GetUniformBufferBinding(const entt::id_type resourceId) const;

   private:
      void AppendShader(ShaderType type, const std::filesystem::path path);
      void ParsePushConstants(spirv_cross::Compiler& compiler);
      void ParseResourceBindings(spirv_cross::Compiler& compiler);
      void LinkShaderProgram();
      void DeleteShaders();
      void FindUniformLocations();

   private:
      std::vector<std::vector<uint32_t>> m_ShaderSrcs;
      std::vector<uint32_t> m_ShaderIds;
      std::unordered_map<entt::id_type, OpenGLUniform> m_PushConstants; // push constants in the Vulkan glsl get turned into uniforms for OpenGL
      std::unordered_map<std::pair<uint32_t, uint32_t>, uint32_t> m_UniformBufferBindingMap; // maps (set, binding) -> open GL binding for uniform buffers
      std::unordered_map<entt::id_type, OpenGLResourceDeclaration> m_UniformBufferResources; // maps resource id (essentially the name of the resource) -> its opengl binding
      std::unordered_map<std::pair<uint32_t, uint32_t>, uint32_t> m_SamplerBindingMap;       // maps (set, binding) -> open GL binding for texture samplers
      std::unordered_map<entt::id_type, OpenGLResourceDeclaration> m_SamplerResources;       // maps resource id (essentially the name of the resource) -> its opengl binding

      uint32_t m_RendererId = 0;
      uint32_t m_VAORendererId = 0;

      inline static std::unordered_map<uint32_t, OpenGLUniformBuffer> s_UniformBuffers;  // there is only one set of uniform buffers over all OpenGL pipelines

   };

}
