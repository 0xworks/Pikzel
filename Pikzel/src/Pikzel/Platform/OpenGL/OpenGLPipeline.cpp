#include "OpenGLPipeline.h"
#include "OpenGLBuffer.h"

#include "Pikzel/Core/Utility.h"

#include <glm/gtc/type_ptr.hpp>
#include <spirv_glsl.hpp>

namespace Pikzel {

   static GLenum DataTypeToOpenGLType(DataType type) {
      switch (type) {
         case DataType::Bool:     return GL_BOOL;
         case DataType::Int:      return GL_INT;
         case DataType::UInt:     return GL_UNSIGNED_INT;
         case DataType::Float:    return GL_FLOAT;
         case DataType::Double:   return GL_DOUBLE;
         case DataType::BVec2:    return GL_BOOL;
         case DataType::BVec3:    return GL_BOOL;
         case DataType::BVec4:    return GL_BOOL;
         case DataType::IVec2:    return GL_INT;
         case DataType::IVec3:    return GL_INT;
         case DataType::IVec4:    return GL_INT;
         case DataType::UVec2:    return GL_UNSIGNED_INT;
         case DataType::UVec3:    return GL_UNSIGNED_INT;
         case DataType::UVec4:    return GL_UNSIGNED_INT;
         case DataType::Vec2:     return GL_FLOAT;
         case DataType::Vec3:     return GL_FLOAT;
         case DataType::Vec4:     return GL_FLOAT;
         case DataType::DVec2:    return GL_DOUBLE;
         case DataType::DVec3:    return GL_DOUBLE;
         case DataType::DVec4:    return GL_DOUBLE;
      }

      PKZL_CORE_ASSERT(false, "Unknown DataType!");
      return 0;
   }


   static GLenum ShaderTypeToOpenGLType(ShaderType type) {
      switch (type) {
         case ShaderType::Vertex:   return GL_VERTEX_SHADER;
         case ShaderType::Geometry: return GL_GEOMETRY_SHADER;
         case ShaderType::Fragment: return GL_FRAGMENT_SHADER;
         case ShaderType::Compute:  return GL_COMPUTE_SHADER;
      }

      PKZL_CORE_ASSERT(false, "Unknown ShaderType!");
      return 0;
   }


   void OpenGLPipeline::SetSpecializationConstants(spirv_cross::Compiler& compiler, const SpecializationConstantsMap& specializationConstants) {
      for (auto& specializationConstant : compiler.get_specialization_constants()) {
         for (const auto& [name, value] : specializationConstants) {
            if (compiler.get_name(specializationConstant.id) == name) {
               auto& constant = compiler.get_constant(specializationConstant.id);
               constant.m.c[0].r[0].i32 = value; // integer constants only for now
               constant.m.c[0].vecsize = 1;
               constant.m.columns = 1;
               constant.specialization = true;
            }
         }
      }
   }


   void OpenGLPipeline::ParsePushConstants(spirv_cross::Compiler& compiler) {
      spirv_cross::ShaderResources resources = compiler.get_shader_resources();
      for (const auto& pushConstantBuffer : resources.push_constant_buffers) {
         const auto& bufferType = compiler.get_type(pushConstantBuffer.base_type_id);
         uint32_t memberCount = static_cast<uint32_t>(bufferType.member_types.size());
         for (uint32_t i = 0; i < memberCount; ++i) {
            std::string uniformName = (pushConstantBuffer.name != "" ? (pushConstantBuffer.name + ".") : "") + compiler.get_member_name(bufferType.self, i);
            const auto& type = compiler.get_type(bufferType.member_types[i]);
            uint32_t offset = compiler.type_struct_member_offset(bufferType, i);
            uint32_t size = static_cast<uint32_t>(compiler.get_declared_struct_member_size(bufferType, i));
            m_PushConstants.try_emplace(entt::hashed_string(uniformName.data()), OpenGLUniform {uniformName, SPIRTypeToDataType(type), -1, size, offset});
         }
      }
   }


   static void ParseResourceBindings_Internal(const std::string_view& resourceType, spirv_cross::Compiler& compiler, OpenGLBindingMap& bindingMap, OpenGLResourceMap& resourceMap, const std::vector<const OpenGLResourceMap*>& otherResourceMaps, spirv_cross::SmallVector<spirv_cross::Resource>& resources) {
      uint32_t numResources = 0;
      for (const auto binding : bindingMap) {
         numResources += binding.second.second;
      }

      for (const auto& resource : resources) {
         const auto& name = resource.name;
         const auto& type = compiler.get_type(resource.type_id);
         const auto& baseType = compiler.get_type(resource.base_type_id);
         std::vector<uint32_t> shape;
         uint32_t count = 1;
         if (type.array.size() > 0) {
            shape.resize(type.array.size());  // number of dimensions of the array. 0 = its a scalar (i.e. not an array), 1 = 1D array, 2 = 2D array, etc...
            for (auto dim = 0; dim < shape.size(); ++dim) {
               shape[dim] = type.array[dim];  // size of [dim]th dimension of the array
               count *= type.array[dim];
            }
         }

         uint32_t set = compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);
         uint32_t binding = compiler.get_decoration(resource.id, spv::DecorationBinding);

         PKZL_CORE_LOG_TRACE("Found {0} at set {1}, binding {2} with name '{3}', count is {4}", resourceType, set, binding, name, count);
         bindingMap.try_emplace(std::make_pair(set, binding), std::make_pair(numResources, count));

         auto [openGLBinding, dummy] = bindingMap.at({set, binding});
         compiler.set_decoration(resource.id, spv::DecorationDescriptorSet, 0);
         compiler.set_decoration(resource.id, spv::DecorationBinding, openGLBinding);
         numResources += count;

         const entt::id_type id = entt::hashed_string(name.data());
         for (const auto otherResourceMap : otherResourceMaps) {
            const auto otherResource = otherResourceMap->find(id);
            if (otherResource != otherResourceMap->end()) {
               throw std::runtime_error {fmt::format("Shader resource name '{0}' is ambiguous.  Refers to different types of resource!", name)};
            }
         }

         const auto openGLResource = resourceMap.find(id);
         if (openGLResource == resourceMap.end()) {
            resourceMap.emplace(entt::hashed_string(name.data()), OpenGLResourceDeclaration {name, openGLBinding, shape});
         } else {
            // already seen this name, check that binding is the same
            if (openGLResource->second.Binding != openGLBinding) {
               throw std::runtime_error {fmt::format("Shader resource name '{0}' is ambiguous.  Refers to different bindings!", name)};
            }
         }
      }

   }

   void OpenGLPipeline::ParseResourceBindings(spirv_cross::Compiler& compiler) {
      spirv_cross::ShaderResources resources = compiler.get_shader_resources();
      ParseResourceBindings_Internal("uniform buffer", compiler, m_UniformBufferBindingMap, m_UniformBufferResources, {&m_SamplerResources, &m_StorageImageResources}, resources.uniform_buffers);
      ParseResourceBindings_Internal("sampler", compiler, m_SamplerBindingMap, m_SamplerResources, {&m_UniformBufferResources, &m_StorageImageResources}, resources.sampled_images);
      ParseResourceBindings_Internal("storage image", compiler, m_StorageImageBindingMap, m_StorageImageResources, {&m_UniformBufferResources, &m_SamplerResources}, resources.storage_images);
   }


   void OpenGLPipeline::FindUniformLocations() {
      glUseProgram(m_RendererId);

      for (auto& [id, uniform] : m_PushConstants) {
         uniform.Location = glGetUniformLocation(m_RendererId, uniform.Name.data());
         if (uniform.Location == -1) {
            PKZL_CORE_LOG_WARN("Could not find uniform location for {0}", uniform.Name);
         }
         PKZL_CORE_LOG_TRACE("Uniform '{0}' is at location {1}", uniform.Name, uniform.Location);
      }

      for (const auto& [id, sampler] : m_SamplerResources) {
         GLint location = glGetUniformLocation(m_RendererId, sampler.Name.data());
         if (location == -1) {
            PKZL_CORE_LOG_WARN("Could not find uniform location for {0}", sampler.Name);
         }
         PKZL_CORE_LOG_TRACE("Uniform '{0}' is at location {1}", sampler.Name, location);
         glUniform1i(location, sampler.Binding);
      }

      for (const auto& [id, image] : m_StorageImageResources) {
         GLint location = glGetUniformLocation(m_RendererId, image.Name.data());
         if (location == -1) {
            PKZL_CORE_LOG_WARN("Could not find uniform location for {0}", image.Name);
         }
         PKZL_CORE_LOG_TRACE("Uniform '{0}' is at location {1}", image.Name, location);
         glUniform1i(location, image.Binding);
      }
   }


   OpenGLPipeline::OpenGLPipeline(const PipelineSettings& settings)
   : m_EnableBlend {settings.enableBlend}
   {
      std::vector<GLuint> shaders;
      for (const auto& [shaderType, src] : settings.shaders) {
         AppendShader(shaderType, src, settings.specializationConstants);
      }

      LinkShaderProgram();
      DeleteShaders();
      FindUniformLocations();

      glCreateVertexArrays(1, &m_VAORendererId);
      glBindVertexArray(m_VAORendererId);

      GLuint vertexAttributeIndex = 0;
      for (const auto& element : settings.bufferLayout) {
         switch (element.dataType) {
            case DataType::Bool:
            case DataType::Int:
            case DataType::UInt:
            case DataType::Float:
            case DataType::Double:
            case DataType::BVec2:
            case DataType::BVec3:
            case DataType::BVec4:
            case DataType::IVec2:
            case DataType::IVec3:
            case DataType::IVec4:
            case DataType::UVec2:
            case DataType::UVec3:
            case DataType::UVec4:
            case DataType::Vec2:
            case DataType::Vec3:
            case DataType::Vec4:
            case DataType::DVec2:
            case DataType::DVec3:
            case DataType::DVec4:
            {
               glEnableVertexAttribArray(vertexAttributeIndex);
               glVertexAttribFormat(vertexAttributeIndex, element.GetComponentCount(), DataTypeToOpenGLType(element.dataType), element.normalized ? GL_TRUE : GL_FALSE, element.offset);
               glVertexAttribBinding(vertexAttributeIndex, 0);
               ++vertexAttributeIndex;
               break;
            }
            case DataType::Mat2:
            case DataType::Mat2x3:
            case DataType::Mat2x4:
            case DataType::Mat3x2:
            case DataType::Mat3:
            case DataType::Mat3x4:
            case DataType::Mat4x2:
            case DataType::Mat4x3:
            case DataType::Mat4:
            case DataType::DMat2:
            case DataType::DMat2x3:
            case DataType::DMat2x4:
            case DataType::DMat3x2:
            case DataType::DMat3:
            case DataType::DMat3x4:
            case DataType::DMat4x2:
            case DataType::DMat4x3:
            case DataType::DMat4:
            {
               uint8_t count = element.GetComponentCount();
               for (uint8_t i = 0; i < count; i++) {
                  glEnableVertexAttribArray(vertexAttributeIndex);
                  glVertexAttribFormat(vertexAttributeIndex, count, DataTypeToOpenGLType(element.dataType), element.normalized ? GL_TRUE : GL_FALSE, static_cast<GLuint>(sizeof(float) * count * i));
                  glVertexAttribDivisor(vertexAttributeIndex, 1);
                  glVertexAttribBinding(vertexAttributeIndex, 0);
                  ++vertexAttributeIndex;
               }
               break;
            }
            default:
               PKZL_CORE_ASSERT(false, "Unknown DataType!");
         }
      }
   }


   OpenGLPipeline::~OpenGLPipeline() {
      glDeleteVertexArrays(1, &m_VAORendererId);
      glDeleteProgram(m_RendererId);
   }


   GLuint OpenGLPipeline::GetRendererId() const {
      return m_RendererId;
   }


   GLuint OpenGLPipeline::GetVAORendererId() const {
      return m_VAORendererId;
   }


   void OpenGLPipeline::PushConstant(const entt::id_type id, bool value) {
      PKZL_CORE_ASSERT(m_PushConstants.at(id).Type == DataType::Bool, "Uniform '{0}' type mismatch.  Bool given, expected {1}!", m_PushConstants.at(id).Name, DataTypeToString(m_PushConstants.at(id).Type));
      glUniform1i(m_PushConstants.at(id).Location, value);
   }


   void OpenGLPipeline::PushConstant(const entt::id_type id, int value) {
      PKZL_CORE_ASSERT(m_PushConstants.at(id).Type == DataType::Int, "Uniform '{0}' type mismatch.  Int given, expected {1}!", m_PushConstants.at(id).Name, DataTypeToString(m_PushConstants.at(id).Type));
      glUniform1i(m_PushConstants.at(id).Location, value);
   }


   void OpenGLPipeline::PushConstant(const entt::id_type id, uint32_t value) {
      PKZL_CORE_ASSERT(m_PushConstants.at(id).Type == DataType::UInt, "Uniform '{0}' type mismatch.  UInt given, expected {1}!", m_PushConstants.at(id).Name, DataTypeToString(m_PushConstants.at(id).Type));
      glUniform1ui(m_PushConstants.at(id).Location, value);
   }


   void OpenGLPipeline::PushConstant(const entt::id_type id, float value) {
      PKZL_CORE_ASSERT(m_PushConstants.at(id).Type == DataType::Float, "Uniform '{0}' type mismatch.  Float given, expected {1}!", m_PushConstants.at(id).Name, DataTypeToString(m_PushConstants.at(id).Type));
      glUniform1f(m_PushConstants.at(id).Location, value);
   }


   void OpenGLPipeline::PushConstant(const entt::id_type id, double value) {
      PKZL_CORE_ASSERT(m_PushConstants.at(id).Type == DataType::Double, "Uniform '{0}' type mismatch.  Double given, expected {1}!", m_PushConstants.at(id).Name, DataTypeToString(m_PushConstants.at(id).Type));
      glUniform1d(m_PushConstants.at(id).Location, value);
   }


   void OpenGLPipeline::PushConstant(const entt::id_type id, const glm::bvec2& value) {
      PKZL_CORE_ASSERT(m_PushConstants.at(id).Type == DataType::BVec2, "Uniform '{0}' type mismatch.  BVec2 given, expected {1}!", m_PushConstants.at(id).Name, DataTypeToString(m_PushConstants.at(id).Type));
      glUniform2i(m_PushConstants.at(id).Location, value.x, value.y);
   }


   void OpenGLPipeline::PushConstant(const entt::id_type id, const glm::bvec3& value) {
      PKZL_CORE_ASSERT(m_PushConstants.at(id).Type == DataType::BVec3, "Uniform '{0}' type mismatch.  BVec3 given, expected {1}!", m_PushConstants.at(id).Name, DataTypeToString(m_PushConstants.at(id).Type));
      glUniform3i(m_PushConstants.at(id).Location, value.x, value.y, value.z);
   }


   void OpenGLPipeline::PushConstant(const entt::id_type id, const glm::bvec4& value) {
      PKZL_CORE_ASSERT(m_PushConstants.at(id).Type == DataType::BVec4, "Uniform '{0}' type mismatch.  BVec4 given, expected {1}!", m_PushConstants.at(id).Name, DataTypeToString(m_PushConstants.at(id).Type));
      glUniform4i(m_PushConstants.at(id).Location, value.x, value.y, value.z, value.w);
   }


   void OpenGLPipeline::PushConstant(const entt::id_type id, const glm::ivec2& value) {
      PKZL_CORE_ASSERT(m_PushConstants.at(id).Type == DataType::IVec2, "Uniform '{0}' type mismatch.  IVec2 given, expected {1}!", m_PushConstants.at(id).Name, DataTypeToString(m_PushConstants.at(id).Type));
      glUniform2iv(m_PushConstants.at(id).Location, 1, glm::value_ptr(value));
   }


   void OpenGLPipeline::PushConstant(const entt::id_type id, const glm::ivec3& value) {
      PKZL_CORE_ASSERT(m_PushConstants.at(id).Type == DataType::IVec3, "Uniform '{0}' type mismatch.  IVec3 given, expected {1}!", m_PushConstants.at(id).Name, DataTypeToString(m_PushConstants.at(id).Type));
      glUniform3iv(m_PushConstants.at(id).Location, 1, glm::value_ptr(value));
   }


   void OpenGLPipeline::PushConstant(const entt::id_type id, const glm::ivec4& value) {
      PKZL_CORE_ASSERT(m_PushConstants.at(id).Type == DataType::IVec4, "Uniform '{0}' type mismatch.  IVec4 given, expected {1}!", m_PushConstants.at(id).Name, DataTypeToString(m_PushConstants.at(id).Type));
      glUniform2iv(m_PushConstants.at(id).Location, 1, glm::value_ptr(value));
   }


   void OpenGLPipeline::PushConstant(const entt::id_type id, const glm::uvec2& value) {
      PKZL_CORE_ASSERT(m_PushConstants.at(id).Type == DataType::UVec2, "Uniform '{0}' type mismatch.  UVec2 given, expected {1}!", m_PushConstants.at(id).Name, DataTypeToString(m_PushConstants.at(id).Type));
      glUniform2uiv(m_PushConstants.at(id).Location, 1, glm::value_ptr(value));

   }


   void OpenGLPipeline::PushConstant(const entt::id_type id, const glm::uvec3& value) {
      PKZL_CORE_ASSERT(m_PushConstants.at(id).Type == DataType::UVec3, "Uniform '{0}' type mismatch.  UVec3 given, expected {1}!", m_PushConstants.at(id).Name, DataTypeToString(m_PushConstants.at(id).Type));
      glUniform3uiv(m_PushConstants.at(id).Location, 1, glm::value_ptr(value));
   }


   void OpenGLPipeline::PushConstant(const entt::id_type id, const glm::uvec4& value) {
      PKZL_CORE_ASSERT(m_PushConstants.at(id).Type == DataType::UVec4, "Uniform '{0}' type mismatch.  UVec4 given, expected {1}!", m_PushConstants.at(id).Name, DataTypeToString(m_PushConstants.at(id).Type));
      glUniform4uiv(m_PushConstants.at(id).Location, 1, glm::value_ptr(value));
   }


   void OpenGLPipeline::PushConstant(const entt::id_type id, const glm::vec2& value) {
      PKZL_CORE_ASSERT(m_PushConstants.at(id).Type == DataType::Vec2, "Uniform '{0}' type mismatch.  Vec2 given, expected {1}!", m_PushConstants.at(id).Name, DataTypeToString(m_PushConstants.at(id).Type));
      glUniform2fv(m_PushConstants.at(id).Location, 1, glm::value_ptr(value));
   }


   void OpenGLPipeline::PushConstant(const entt::id_type id, const glm::vec3& value) {
      PKZL_CORE_ASSERT(m_PushConstants.at(id).Type == DataType::Vec3, "Uniform '{0}' type mismatch.  Vec3 given, expected {1}!", m_PushConstants.at(id).Name, DataTypeToString(m_PushConstants.at(id).Type));
      glUniform3fv(m_PushConstants.at(id).Location, 1, glm::value_ptr(value));
   }


   void OpenGLPipeline::PushConstant(const entt::id_type id, const glm::vec4& value) {
      PKZL_CORE_ASSERT(m_PushConstants.at(id).Type == DataType::Vec4, "Uniform '{0}' type mismatch.  Vec4 given, expected {1}!", m_PushConstants.at(id).Name, DataTypeToString(m_PushConstants.at(id).Type));
      glUniform4fv(m_PushConstants.at(id).Location, 1, glm::value_ptr(value));
   }


   void OpenGLPipeline::PushConstant(const entt::id_type id, const glm::dvec2& value) {
      PKZL_CORE_ASSERT(m_PushConstants.at(id).Type == DataType::DVec2, "Uniform '{0}' type mismatch.  DVec2 given, expected {1}!", m_PushConstants.at(id).Name, DataTypeToString(m_PushConstants.at(id).Type));
      glUniform2dv(m_PushConstants.at(id).Location, 1, glm::value_ptr(value));
   }


   void OpenGLPipeline::PushConstant(const entt::id_type id, const glm::dvec3& value) {
      PKZL_CORE_ASSERT(m_PushConstants.at(id).Type == DataType::DVec3, "Uniform '{0}' type mismatch.  DVec3 given, expected {1}!", m_PushConstants.at(id).Name, DataTypeToString(m_PushConstants.at(id).Type));
      glUniform3dv(m_PushConstants.at(id).Location, 1, glm::value_ptr(value));
   }


   void OpenGLPipeline::PushConstant(const entt::id_type id, const glm::dvec4& value) {
      PKZL_CORE_ASSERT(m_PushConstants.at(id).Type == DataType::DVec4, "Uniform '{0}' type mismatch.  DVec4 given, expected {1}!", m_PushConstants.at(id).Name, DataTypeToString(m_PushConstants.at(id).Type));
      glUniform4dv(m_PushConstants.at(id).Location, 1, glm::value_ptr(value));
   }


   void OpenGLPipeline::PushConstant(const entt::id_type id, const glm::mat2& value) {
      PKZL_CORE_ASSERT(m_PushConstants.at(id).Type == DataType::Mat2, "Uniform '{0}' type mismatch.  Mat2 given, expected {1}!", m_PushConstants.at(id).Name, DataTypeToString(m_PushConstants.at(id).Type));
      glUniformMatrix2fv(m_PushConstants.at(id).Location, 1, false, glm::value_ptr(value));
   }


   //void OpenGLPipeline::PushConstant(const entt::id_type id, const glm::mat2x3& value) {
   //   PKZL_CORE_ASSERT(m_PushConstants.at(id).Type == DataType::Mat2x3, "Uniform '{0}' type mismatch.  Mat2x3 given, expected {1}!", m_PushConstants.at(id).Name, DataTypeToString(m_PushConstants.at(id).Type));
   //   glUniformMatrix2x3fv(m_PushConstants.at(id).Location, 1, false, glm::value_ptr(value));
   //}


   void OpenGLPipeline::PushConstant(const entt::id_type id, const glm::mat2x4& value) {
      PKZL_CORE_ASSERT(m_PushConstants.at(id).Type == DataType::Mat2x4, "Uniform '{0}' type mismatch.  Mat2x4 given, expected {1}!", m_PushConstants.at(id).Name, DataTypeToString(m_PushConstants.at(id).Type));
      glUniformMatrix2x4fv(m_PushConstants.at(id).Location, 1, false, glm::value_ptr(value));
   }


   void OpenGLPipeline::PushConstant(const entt::id_type id, const glm::mat3x2& value) {
      PKZL_CORE_ASSERT(m_PushConstants.at(id).Type == DataType::Mat3x2, "Uniform '{0}' type mismatch.  Mat3x2 given, expected {1}!", m_PushConstants.at(id).Name, DataTypeToString(m_PushConstants.at(id).Type));
      glUniformMatrix3x2fv(m_PushConstants.at(id).Location, 1, false, glm::value_ptr(value));
   }


   //void OpenGLPipeline::PushConstant(const entt::id_type id, const glm::mat3& value) {
   //   PKZL_CORE_ASSERT(m_PushConstants.at(id).Type == DataType::Mat3, "Uniform '{0}' type mismatch.  Mat3 given, expected {1}!", m_PushConstants.at(id).Name, DataTypeToString(m_PushConstants.at(id).Type));
   //   glUniformMatrix3fv(m_PushConstants.at(id).Location, 1, false, glm::value_ptr(value));
   //}


   void OpenGLPipeline::PushConstant(const entt::id_type id, const glm::mat3x4& value) {
      PKZL_CORE_ASSERT(m_PushConstants.at(id).Type == DataType::Mat3x4, "Uniform '{0}' type mismatch.  Mat3x4 given, expected {1}!", m_PushConstants.at(id).Name, DataTypeToString(m_PushConstants.at(id).Type));
      glUniformMatrix3x4fv(m_PushConstants.at(id).Location, 1, false, glm::value_ptr(value));
   }


   void OpenGLPipeline::PushConstant(const entt::id_type id, const glm::mat4x2& value) {
      PKZL_CORE_ASSERT(m_PushConstants.at(id).Type == DataType::Mat4x2, "Uniform '{0}' type mismatch.  Mat4x2 given, expected {1}!", m_PushConstants.at(id).Name, DataTypeToString(m_PushConstants.at(id).Type));
      glUniformMatrix4x2fv(m_PushConstants.at(id).Location, 1, false, glm::value_ptr(value));
   }


   //void OpenGLPipeline::PushConstant(const entt::id_type id, const glm::mat4x3& value) {
   //   PKZL_CORE_ASSERT(m_PushConstants.at(id).Type == DataType::Mat4x3, "Uniform '{0}' type mismatch.  Mat4x3 given, expected {1}!", m_PushConstants.at(id).Name, DataTypeToString(m_PushConstants.at(id).Type));
   //   glUniformMatrix4x3fv(m_PushConstants.at(id).Location, 1, false, glm::value_ptr(value));
   //}


   void OpenGLPipeline::PushConstant(const entt::id_type id, const glm::mat4& value) {
      PKZL_CORE_ASSERT(m_PushConstants.at(id).Type == DataType::Mat4, "Uniform '{0}' type mismatch.  Mat4 given, expected {1}!", m_PushConstants.at(id).Name, DataTypeToString(m_PushConstants.at(id).Type));
      glUniformMatrix4fv(m_PushConstants.at(id).Location, 1, GL_FALSE, glm::value_ptr(value));
   }


   void OpenGLPipeline::PushConstant(const entt::id_type id, const glm::dmat2& value) {
      PKZL_CORE_ASSERT(m_PushConstants.at(id).Type == DataType::DMat2, "Uniform '{0}' type mismatch.  DMat2 given, expected {1}!", m_PushConstants.at(id).Name, DataTypeToString(m_PushConstants.at(id).Type));
      glUniformMatrix2dv(m_PushConstants.at(id).Location, 1, false, glm::value_ptr(value));
   }


   //void OpenGLPipeline::PushConstant(const entt::id_type id, const glm::dmat2x3& value) {
   //   PKZL_CORE_ASSERT(m_PushConstants.at(id).Type == DataType::DMat2x3, "Uniform '{0}' type mismatch.  DMat2x3 given, expected {1}!", m_PushConstants.at(id).Name, DataTypeToString(m_PushConstants.at(id).Type));
   //   glUniformMatrix2x3dv(m_PushConstants.at(id).Location, 1, false, glm::value_ptr(value));
   //}


   void OpenGLPipeline::PushConstant(const entt::id_type id, const glm::dmat2x4& value) {
      PKZL_CORE_ASSERT(m_PushConstants.at(id).Type == DataType::DMat2x4, "Uniform '{0}' type mismatch.  DMat2x4 given, expected {1}!", m_PushConstants.at(id).Name, DataTypeToString(m_PushConstants.at(id).Type));
      glUniformMatrix2x4dv(m_PushConstants.at(id).Location, 1, false, glm::value_ptr(value));
   }


   void OpenGLPipeline::PushConstant(const entt::id_type id, const glm::dmat3x2& value) {
      PKZL_CORE_ASSERT(m_PushConstants.at(id).Type == DataType::DMat3x2, "Uniform '{0}' type mismatch.  DMat3x2 given, expected {1}!", m_PushConstants.at(id).Name, DataTypeToString(m_PushConstants.at(id).Type));
      glUniformMatrix3x2dv(m_PushConstants.at(id).Location, 1, false, glm::value_ptr(value));
   }


   //void OpenGLPipeline::PushConstant(const entt::id_type id, const glm::dmat3& value) {
   //   PKZL_CORE_ASSERT(m_PushConstants.at(id).Type == DataType::DMat3, "Uniform '{0}' type mismatch.  DMat3 given, expected {1}!", m_PushConstants.at(id).Name, DataTypeToString(m_PushConstants.at(id).Type));
   //   glUniformMatrix3dv(m_PushConstants.at(id).Location, 1, false, glm::value_ptr(value));
   //}


   void OpenGLPipeline::PushConstant(const entt::id_type id, const glm::dmat3x4& value) {
      PKZL_CORE_ASSERT(m_PushConstants.at(id).Type == DataType::DMat3x4, "Uniform '{0}' type mismatch.  DMat3x4 given, expected {1}!", m_PushConstants.at(id).Name, DataTypeToString(m_PushConstants.at(id).Type));
      glUniformMatrix3x4dv(m_PushConstants.at(id).Location, 1, false, glm::value_ptr(value));
   }


   void OpenGLPipeline::PushConstant(const entt::id_type id, const glm::dmat4x2& value) {
      PKZL_CORE_ASSERT(m_PushConstants.at(id).Type == DataType::DMat4x2, "Uniform '{0}' type mismatch.  DMat4x2 given, expected {1}!", m_PushConstants.at(id).Name, DataTypeToString(m_PushConstants.at(id).Type));
      glUniformMatrix4x2dv(m_PushConstants.at(id).Location, 1, false, glm::value_ptr(value));
   }


   //void OpenGLPipeline::PushConstant(const entt::id_type id, const glm::dmat4x3& value) {
   //   PKZL_CORE_ASSERT(m_PushConstants.at(id).Type == DataType::DMat4x3, "Uniform '{0}' type mismatch.  DMat4x3 given, expected {1}!", m_PushConstants.at(id).Name, DataTypeToString(m_PushConstants.at(id).Type));
   //   glUniformMatrix4x3dv(m_PushConstants.at(id).Location, 1, false, glm::value_ptr(value));
   //}


   void OpenGLPipeline::PushConstant(const entt::id_type id, const glm::dmat4& value) {
      PKZL_CORE_ASSERT(m_PushConstants.at(id).Type == DataType::DMat4, "Uniform '{0}' type mismatch.  DMat4 given, expected {1}!", m_PushConstants.at(id).Name, DataTypeToString(m_PushConstants.at(id).Type));
      glUniformMatrix4dv(m_PushConstants.at(id).Location, 1, GL_FALSE, glm::value_ptr(value));
   }


   GLuint OpenGLPipeline::GetSamplerBinding(const entt::id_type resourceId, const bool exceptionIfNotFound) const {
      const auto resource = m_SamplerResources.find(resourceId);
      GLuint retVal = (resource == m_SamplerResources.end()) ? ~0 : resource->second.Binding;
      if (exceptionIfNotFound && retVal == ~0) {
         throw std::invalid_argument {fmt::format("OpenGLPipeline::GetSamplerBinding() failed to find resource with id {0}", resourceId)};
      }
      return retVal;
   }


   GLuint OpenGLPipeline::GetStorageImageBinding(const entt::id_type resourceId, bool exceptionIfNotFound) const {
      const auto resource = m_StorageImageResources.find(resourceId);
      GLuint retVal = (resource == m_StorageImageResources.end()) ? ~0 : resource->second.Binding;
      if (exceptionIfNotFound && retVal == ~0) {
         throw std::invalid_argument {fmt::format("OpenGLPipeline::GetStorageImageResources() failed to find resource with id {0}!", resourceId)};
      }
      return retVal;
   }


   GLuint OpenGLPipeline::GetUniformBufferBinding(const entt::id_type resourceId, bool exceptionIfNotFound) const {
      const auto resource = m_UniformBufferResources.find(resourceId);
      GLuint retVal = (resource == m_UniformBufferResources.end()) ? ~0 : resource->second.Binding;
      if (exceptionIfNotFound && retVal == ~0) {
         throw std::invalid_argument {fmt::format("OpenGLPipeline::GetUniformBufferBinding() failed to find resource with id {0}!", resourceId)};
      }
      return retVal;
   }


   void OpenGLPipeline::SetGLState() const {
      glUseProgram(GetRendererId());
      glBindVertexArray(GetVAORendererId());
      if (m_EnableBlend) {
         glEnable(GL_BLEND);
      } else {
         glDisable(GL_BLEND);
      }
   }


   void OpenGLPipeline::AppendShader(ShaderType type, const std::filesystem::path path, const SpecializationConstantsMap& specializationConstants) {
      PKZL_CORE_LOG_TRACE("Appending shader '{0}'", path.string());

      std::vector<uint32_t> src = ReadFile<uint32_t>(path);

      spirv_cross::CompilerGLSL compiler(src);
      ParsePushConstants(compiler);
      ParseResourceBindings(compiler);
      SetSpecializationConstants(compiler, specializationConstants);
      std::string glsl = compiler.compile();

      GLuint shader = glCreateShader(ShaderTypeToOpenGLType(type));
      const GLchar* srcC = glsl.data();
      glShaderSource(shader, 1, &srcC, nullptr);
      glCompileShader(shader);

      GLint isCompiled = 0;
      glGetShaderiv(shader, GL_COMPILE_STATUS, &isCompiled);
      if (isCompiled == GL_FALSE) {
         GLint maxLength = 0;
         glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);

         std::vector<GLchar> infoLog(maxLength);
         glGetShaderInfoLog(shader, maxLength, &maxLength, &infoLog[0]);

         glDeleteShader(shader);

         PKZL_CORE_LOG_ERROR("{0}", infoLog.data());
         throw std::runtime_error {"Shader compilation failure!"};
      }

      m_ShaderIds.emplace_back(shader);
   }


   void OpenGLPipeline::LinkShaderProgram() {
      if (m_RendererId != 0) {
         glDeleteProgram(m_RendererId);
         m_RendererId = 0;
      }
      m_RendererId = glCreateProgram();

      for (const auto shaderId : m_ShaderIds) {
         glAttachShader(m_RendererId, shaderId);
      }

      glLinkProgram(m_RendererId);

      GLint isLinked = 0;
      glGetProgramiv(m_RendererId, GL_LINK_STATUS, &isLinked);
      if (isLinked == GL_FALSE) {
         GLint maxLength = 0;
         glGetProgramiv(m_RendererId, GL_INFO_LOG_LENGTH, &maxLength);

         std::vector<GLchar> infoLog(maxLength);
         glGetProgramInfoLog(m_RendererId, maxLength, &maxLength, &infoLog[0]);

         glDeleteProgram(m_RendererId);
         for (const auto shaderId : m_ShaderIds) {
            glDeleteShader(shaderId);
         }

         PKZL_CORE_LOG_ERROR("{0}", infoLog.data());
         throw std::runtime_error {"Shader link failure!"};
      }

      for (const auto shaderId : m_ShaderIds) {
         glDetachShader(m_RendererId, shaderId);
      }
   }


   void OpenGLPipeline::DeleteShaders() {
      for (const auto shaderId : m_ShaderIds) {
         glDeleteShader(shaderId);
      }
      m_ShaderIds.clear();
      m_ShaderSrcs.clear();
   }
}
