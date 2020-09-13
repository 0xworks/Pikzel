#pragma once

#include "Pikzel/Renderer/GraphicsContext.h"
#include "Pikzel/Renderer/Pipeline.h"
#include "Pikzel/Renderer/ShaderUtil.h"

#include <glm/glm.hpp>

#include <unordered_map>
#include <vector>

namespace Pikzel {

   struct ShaderUniform {
      std::string Name;
      DataType Type = DataType::None;
      int Location = 0;
      uint32_t Offset = 0;
      uint32_t Size = 0;
   };


   struct ShaderUniformBuffer {
      std::string Name;
      uint32_t BindingPoint = 0;
      uint32_t Size = 0;
      uint32_t RendererId = 0;
      std::vector<ShaderUniform> Uniforms;
   };


   struct ShaderResourceDeclaration {
      std::string Name;
      uint32_t BindingPoint = 0;
      uint32_t m_Count = 0;
   };


   class OpenGLPipeline : public Pipeline {
   public:
      OpenGLPipeline(GraphicsContext& gc, const PipelineSettings& settings);
      virtual ~OpenGLPipeline();

      GLuint GetRendererId() const;
      GLuint GetVAORendererId() const;

      void PushConstant(const std::string& name, bool value);
      void PushConstant(const std::string& name, int value);
      void PushConstant(const std::string& name, uint32_t value);
      void PushConstant(const std::string & name, float value);
      void PushConstant(const std::string& name, double value);
      void PushConstant(const std::string& name, const glm::bvec2& value);
      void PushConstant(const std::string& name, const glm::bvec3& value);
      void PushConstant(const std::string& name, const glm::bvec4& value);
      void PushConstant(const std::string& name, const glm::ivec2& value);
      void PushConstant(const std::string& name, const glm::ivec3& value);
      void PushConstant(const std::string& name, const glm::ivec4& value);
      void PushConstant(const std::string& name, const glm::uvec2& value);
      void PushConstant(const std::string& name, const glm::uvec3& value);
      void PushConstant(const std::string& name, const glm::uvec4& value);
      void PushConstant(const std::string& name, const glm::vec2& value);
      void PushConstant(const std::string& name, const glm::vec3& value);
      void PushConstant(const std::string& name, const glm::vec4& value);
      void PushConstant(const std::string& name, const glm::dvec2& value);
      void PushConstant(const std::string& name, const glm::dvec3& value);
      void PushConstant(const std::string& name, const glm::dvec4& value);
      void PushConstant(const std::string& name, const glm::mat2& value);
      void PushConstant(const std::string& name, const glm::mat2x3& value);
      void PushConstant(const std::string& name, const glm::mat2x4& value);
      void PushConstant(const std::string& name, const glm::mat3x2& value);
      void PushConstant(const std::string& name, const glm::mat3& value);
      void PushConstant(const std::string& name, const glm::mat3x4& value);
      void PushConstant(const std::string& name, const glm::mat4x2& value);
      void PushConstant(const std::string& name, const glm::mat4x3& value);
      void PushConstant(const std::string& name, const glm::mat4& value);
      void PushConstant(const std::string& name, const glm::dmat2& value);
      void PushConstant(const std::string& name, const glm::dmat2x3& value);
      void PushConstant(const std::string& name, const glm::dmat2x4& value);
      void PushConstant(const std::string& name, const glm::dmat3x2& value);
      void PushConstant(const std::string& name, const glm::dmat3& value);
      void PushConstant(const std::string& name, const glm::dmat3x4& value);
      void PushConstant(const std::string& name, const glm::dmat4x2& value);
      void PushConstant(const std::string& name, const glm::dmat4x3& value);
      void PushConstant(const std::string& name, const glm::dmat4& value);

   private:
      void AppendShader(ShaderType type, const std::filesystem::path path);
      void LinkShaderProgram();
      void ReflectShaders();
      void DeleteShaders();

   private:
      std::vector<std::vector<uint32_t>> m_ShaderSrcs;
      std::vector<uint32_t> m_ShaderIds;
      std::unordered_map<std::string, ShaderUniform> m_PushConstants; // push constants in the Vulkan glsl get turned into uniforms for OpenGL
      std::unordered_map<std::string, ShaderResourceDeclaration> m_Resources; // e.g. image samplers

      uint32_t m_RendererId = 0;
      uint32_t m_VAORendererId = 0;

      inline static std::unordered_map<uint32_t, ShaderUniformBuffer> s_UniformBuffers;  // there is only one set of uniform buffers over all OpenGL pipelines

   };

}
