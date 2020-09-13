#include "glpch.h"
#include "OpenGLPipeline.h"
#include "OpenGLBuffer.h"

#include "Pikzel/Core/Utility.h"

#include <glm/gtc/type_ptr.hpp>
#include <spirv_cross/spirv_glsl.hpp>

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
         case ShaderType::Fragment: return GL_FRAGMENT_SHADER;
      }

      PKZL_CORE_ASSERT(false, "Unknown ShaderType!");
      return 0;
   }


   static std::unordered_map<std::string, ShaderUniform> ParsePushConstants(spirv_cross::Compiler& compiler) {
      std::unordered_map<std::string, ShaderUniform> pushConstants;
      spirv_cross::ShaderResources resources = compiler.get_shader_resources();
      for (const auto& pushConstantBuffer : resources.push_constant_buffers) {
         PKZL_CORE_LOG_TRACE("Found push constant with name '{0}'", pushConstantBuffer.name);
         const auto& bufferType = compiler.get_type(pushConstantBuffer.base_type_id);
         uint32_t memberCount = static_cast<uint32_t>(bufferType.member_types.size());
         for (uint32_t i = 0; i < memberCount; ++i) {
            std::string uniformName = (pushConstantBuffer.name != "" ? (pushConstantBuffer.name + ".") : "") + compiler.get_member_name(bufferType.self, i);
            const auto& type = compiler.get_type(bufferType.member_types[i]);
            uint32_t offset = compiler.type_struct_member_offset(bufferType, i);
            uint32_t size = static_cast<uint32_t>(compiler.get_declared_struct_member_size(bufferType, i));
            pushConstants.emplace(std::make_pair(uniformName, ShaderUniform {uniformName, SPIRTypeToDataType(type), -1, size, offset}));
         }
      }
      return pushConstants;
   }


   OpenGLPipeline::OpenGLPipeline(GraphicsContext& gc, const PipelineSettings& settings) {
      std::vector<GLuint> shaders;
      for (const auto& [shaderType, src] : settings.Shaders) {
         AppendShader(shaderType, src);
      }

      LinkShaderProgram();
      ReflectShaders();
      DeleteShaders();

      glCreateVertexArrays(1, &m_VAORendererId);
      glBindVertexArray(m_VAORendererId);
      GCBinder bind {gc, settings.VertexBuffer};

      GLuint vertexAttributeIndex = 0;
      for (const auto& element : settings.VertexBuffer.GetLayout()) {
         switch (element.Type) {
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
               glVertexAttribPointer(vertexAttributeIndex, element.GetComponentCount(), DataTypeToOpenGLType(element.Type), element.Normalized ? GL_TRUE : GL_FALSE, settings.VertexBuffer.GetLayout().GetStride(), (const void*)element.Offset);
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
                  glVertexAttribPointer(vertexAttributeIndex, count, DataTypeToOpenGLType(element.Type), element.Normalized ? GL_TRUE : GL_FALSE, settings.VertexBuffer.GetLayout().GetStride(), (const void*)(sizeof(float) * count * i));
                  glVertexAttribDivisor(vertexAttributeIndex, 1);
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


   void OpenGLPipeline::PushConstant(const std::string& name, bool value) {
      PKZL_CORE_ASSERT(m_PushConstants[name].Type == DataType::Bool, "Uniform '{0}' type mismatch.  Bool given, expected {1}!", name, DataTypeToString(m_PushConstants[name].Type));
      glUniform1i(m_PushConstants[name].Location, value);
   }


   void OpenGLPipeline::PushConstant(const std::string& name, int value) {
      PKZL_CORE_ASSERT(m_PushConstants[name].Type == DataType::Int, "Uniform '{0}' type mismatch.  Int given, expected {1}!", name, DataTypeToString(m_PushConstants[name].Type));
      glUniform1i(m_PushConstants[name].Location, value);
   }


   void OpenGLPipeline::PushConstant(const std::string& name, uint32_t value) {
      PKZL_CORE_ASSERT(m_PushConstants[name].Type == DataType::UInt, "Uniform '{0}' type mismatch.  UInt given, expected {1}!", name, DataTypeToString(m_PushConstants[name].Type));
      glUniform1ui(m_PushConstants[name].Location, value);
   }


   void OpenGLPipeline::PushConstant(const std::string& name, float value) {
      PKZL_CORE_ASSERT(m_PushConstants[name].Type == DataType::Float, "Uniform '{0}' type mismatch.  Float given, expected {1}!", name, DataTypeToString(m_PushConstants[name].Type));
      glUniform1f(m_PushConstants[name].Location, value);
   }


   void OpenGLPipeline::PushConstant(const std::string& name, double value) {
      PKZL_CORE_ASSERT(m_PushConstants[name].Type == DataType::Double, "Uniform '{0}' type mismatch.  Double given, expected {1}!", name, DataTypeToString(m_PushConstants[name].Type));
      glUniform1d(m_PushConstants[name].Location, value);
   }


   void OpenGLPipeline::PushConstant(const std::string& name, const glm::bvec2& value) {
      PKZL_CORE_ASSERT(m_PushConstants[name].Type == DataType::BVec2, "Uniform '{0}' type mismatch.  BVec2 given, expected {1}!", name, DataTypeToString(m_PushConstants[name].Type));
      glUniform2i(m_PushConstants[name].Location, value.x, value.y);
   }


   void OpenGLPipeline::PushConstant(const std::string& name, const glm::bvec3& value) {
      PKZL_CORE_ASSERT(m_PushConstants[name].Type == DataType::BVec3, "Uniform '{0}' type mismatch.  BVec3 given, expected {1}!", name, DataTypeToString(m_PushConstants[name].Type));
      glUniform3i(m_PushConstants[name].Location, value.x, value.y, value.z);
   }


   void OpenGLPipeline::PushConstant(const std::string& name, const glm::bvec4& value) {
      PKZL_CORE_ASSERT(m_PushConstants[name].Type == DataType::BVec4, "Uniform '{0}' type mismatch.  BVec4 given, expected {1}!", name, DataTypeToString(m_PushConstants[name].Type));
      glUniform4i(m_PushConstants[name].Location, value.x, value.y, value.z, value.w);
   }


   void OpenGLPipeline::PushConstant(const std::string& name, const glm::ivec2& value) {
      PKZL_CORE_ASSERT(m_PushConstants[name].Type == DataType::IVec2, "Uniform '{0}' type mismatch.  IVec2 given, expected {1}!", name, DataTypeToString(m_PushConstants[name].Type));
      glUniform2iv(m_PushConstants[name].Location, 1, glm::value_ptr(value));
   }


   void OpenGLPipeline::PushConstant(const std::string& name, const glm::ivec3& value) {
      PKZL_CORE_ASSERT(m_PushConstants[name].Type == DataType::IVec3, "Uniform '{0}' type mismatch.  IVec3 given, expected {1}!", name, DataTypeToString(m_PushConstants[name].Type));
      glUniform3iv(m_PushConstants[name].Location, 1, glm::value_ptr(value));
   }


   void OpenGLPipeline::PushConstant(const std::string& name, const glm::ivec4& value) {
      PKZL_CORE_ASSERT(m_PushConstants[name].Type == DataType::IVec4, "Uniform '{0}' type mismatch.  IVec4 given, expected {1}!", name, DataTypeToString(m_PushConstants[name].Type));
      glUniform2iv(m_PushConstants[name].Location, 1, glm::value_ptr(value));
   }


   void OpenGLPipeline::PushConstant(const std::string& name, const glm::uvec2& value) {
      PKZL_CORE_ASSERT(m_PushConstants[name].Type == DataType::UVec2, "Uniform '{0}' type mismatch.  UVec2 given, expected {1}!", name, DataTypeToString(m_PushConstants[name].Type));
      glUniform2uiv(m_PushConstants[name].Location, 1, glm::value_ptr(value));

   }


   void OpenGLPipeline::PushConstant(const std::string& name, const glm::uvec3& value) {
      PKZL_CORE_ASSERT(m_PushConstants[name].Type == DataType::UVec3, "Uniform '{0}' type mismatch.  UVec3 given, expected {1}!", name, DataTypeToString(m_PushConstants[name].Type));
      glUniform3uiv(m_PushConstants[name].Location, 1, glm::value_ptr(value));
   }


   void OpenGLPipeline::PushConstant(const std::string& name, const glm::uvec4& value) {
      PKZL_CORE_ASSERT(m_PushConstants[name].Type == DataType::UVec4, "Uniform '{0}' type mismatch.  UVec4 given, expected {1}!", name, DataTypeToString(m_PushConstants[name].Type));
      glUniform4uiv(m_PushConstants[name].Location, 1, glm::value_ptr(value));
   }


   void OpenGLPipeline::PushConstant(const std::string& name, const glm::vec2& value) {
      PKZL_CORE_ASSERT(m_PushConstants[name].Type == DataType::Vec2, "Uniform '{0}' type mismatch.  Vec2 given, expected {1}!", name, DataTypeToString(m_PushConstants[name].Type));
      glUniform2fv(m_PushConstants[name].Location, 1, glm::value_ptr(value));
   }


   void OpenGLPipeline::PushConstant(const std::string& name, const glm::vec3& value) {
      PKZL_CORE_ASSERT(m_PushConstants[name].Type == DataType::Vec3, "Uniform '{0}' type mismatch.  Vec3 given, expected {1}!", name, DataTypeToString(m_PushConstants[name].Type));
      glUniform3fv(m_PushConstants[name].Location, 1, glm::value_ptr(value));
   }


   void OpenGLPipeline::PushConstant(const std::string& name, const glm::vec4& value) {
      PKZL_CORE_ASSERT(m_PushConstants[name].Type == DataType::Vec4, "Uniform '{0}' type mismatch.  Vec4 given, expected {1}!", name, DataTypeToString(m_PushConstants[name].Type));
      glUniform4fv(m_PushConstants[name].Location, 1, glm::value_ptr(value));
   }


   void OpenGLPipeline::PushConstant(const std::string& name, const glm::dvec2& value) {
      PKZL_CORE_ASSERT(m_PushConstants[name].Type == DataType::DVec2, "Uniform '{0}' type mismatch.  DVec2 given, expected {1}!", name, DataTypeToString(m_PushConstants[name].Type));
      glUniform2dv(m_PushConstants[name].Location, 1, glm::value_ptr(value));
   }


   void OpenGLPipeline::PushConstant(const std::string& name, const glm::dvec3& value) {
      PKZL_CORE_ASSERT(m_PushConstants[name].Type == DataType::DVec3, "Uniform '{0}' type mismatch.  DVec3 given, expected {1}!", name, DataTypeToString(m_PushConstants[name].Type));
      glUniform3dv(m_PushConstants[name].Location, 1, glm::value_ptr(value));
   }


   void OpenGLPipeline::PushConstant(const std::string& name, const glm::dvec4& value) {
      PKZL_CORE_ASSERT(m_PushConstants[name].Type == DataType::DVec4, "Uniform '{0}' type mismatch.  DVec4 given, expected {1}!", name, DataTypeToString(m_PushConstants[name].Type));
      glUniform4dv(m_PushConstants[name].Location, 1, glm::value_ptr(value));
   }


   void OpenGLPipeline::PushConstant(const std::string& name, const glm::mat2& value) {
      PKZL_CORE_ASSERT(m_PushConstants[name].Type == DataType::Mat2, "Uniform '{0}' type mismatch.  Mat2 given, expected {1}!", name, DataTypeToString(m_PushConstants[name].Type));
      glUniformMatrix2fv(m_PushConstants[name].Location, 1, false, glm::value_ptr(value));
   }


   void OpenGLPipeline::PushConstant(const std::string& name, const glm::mat2x3& value) {
      PKZL_CORE_ASSERT(m_PushConstants[name].Type == DataType::Mat2x3, "Uniform '{0}' type mismatch.  Mat2x3 given, expected {1}!", name, DataTypeToString(m_PushConstants[name].Type));
      glUniformMatrix2x3fv(m_PushConstants[name].Location, 1, false, glm::value_ptr(value));
   }


   void OpenGLPipeline::PushConstant(const std::string& name, const glm::mat2x4& value) {
      PKZL_CORE_ASSERT(m_PushConstants[name].Type == DataType::Mat2x4, "Uniform '{0}' type mismatch.  Mat2x4 given, expected {1}!", name, DataTypeToString(m_PushConstants[name].Type));
      glUniformMatrix2x4fv(m_PushConstants[name].Location, 1, false, glm::value_ptr(value));
   }


   void OpenGLPipeline::PushConstant(const std::string& name, const glm::mat3x2& value) {
      PKZL_CORE_ASSERT(m_PushConstants[name].Type == DataType::Mat3x2, "Uniform '{0}' type mismatch.  Mat3x2 given, expected {1}!", name, DataTypeToString(m_PushConstants[name].Type));
      glUniformMatrix3x2fv(m_PushConstants[name].Location, 1, false, glm::value_ptr(value));
   }


   void OpenGLPipeline::PushConstant(const std::string& name, const glm::mat3& value) {
      PKZL_CORE_ASSERT(m_PushConstants[name].Type == DataType::Mat3, "Uniform '{0}' type mismatch.  Mat3 given, expected {1}!", name, DataTypeToString(m_PushConstants[name].Type));
      glUniformMatrix3fv(m_PushConstants[name].Location, 1, false, glm::value_ptr(value));
   }


   void OpenGLPipeline::PushConstant(const std::string& name, const glm::mat3x4& value) {
      PKZL_CORE_ASSERT(m_PushConstants[name].Type == DataType::Mat3x4, "Uniform '{0}' type mismatch.  Mat3x4 given, expected {1}!", name, DataTypeToString(m_PushConstants[name].Type));
      glUniformMatrix3x4fv(m_PushConstants[name].Location, 1, false, glm::value_ptr(value));
   }


   void OpenGLPipeline::PushConstant(const std::string& name, const glm::mat4x2& value) {
      PKZL_CORE_ASSERT(m_PushConstants[name].Type == DataType::Mat4x2, "Uniform '{0}' type mismatch.  Mat4x2 given, expected {1}!", name, DataTypeToString(m_PushConstants[name].Type));
      glUniformMatrix4x2fv(m_PushConstants[name].Location, 1, false, glm::value_ptr(value));
   }


   void OpenGLPipeline::PushConstant(const std::string& name, const glm::mat4x3& value) {
      PKZL_CORE_ASSERT(m_PushConstants[name].Type == DataType::Mat4x3, "Uniform '{0}' type mismatch.  Mat4x3 given, expected {1}!", name, DataTypeToString(m_PushConstants[name].Type));
      glUniformMatrix4x3fv(m_PushConstants[name].Location, 1, false, glm::value_ptr(value));
   }


   void OpenGLPipeline::PushConstant(const std::string& name, const glm::mat4& value) {
      PKZL_CORE_ASSERT(m_PushConstants[name].Type == DataType::Mat4, "Uniform '{0}' type mismatch.  Mat4 given, expected {1}!", name, DataTypeToString(m_PushConstants[name].Type));
      glUniformMatrix4fv(m_PushConstants[name].Location, 1, GL_FALSE, glm::value_ptr(value));
   }


   void OpenGLPipeline::PushConstant(const std::string& name, const glm::dmat2& value) {
      PKZL_CORE_ASSERT(m_PushConstants[name].Type == DataType::DMat2, "Uniform '{0}' type mismatch.  DMat2 given, expected {1}!", name, DataTypeToString(m_PushConstants[name].Type));
      glUniformMatrix2dv(m_PushConstants[name].Location, 1, false, glm::value_ptr(value));
   }


   void OpenGLPipeline::PushConstant(const std::string& name, const glm::dmat2x3& value) {
      PKZL_CORE_ASSERT(m_PushConstants[name].Type == DataType::DMat2x3, "Uniform '{0}' type mismatch.  DMat2x3 given, expected {1}!", name, DataTypeToString(m_PushConstants[name].Type));
      glUniformMatrix2x3dv(m_PushConstants[name].Location, 1, false, glm::value_ptr(value));
   }


   void OpenGLPipeline::PushConstant(const std::string& name, const glm::dmat2x4& value) {
      PKZL_CORE_ASSERT(m_PushConstants[name].Type == DataType::DMat2x4, "Uniform '{0}' type mismatch.  DMat2x4 given, expected {1}!", name, DataTypeToString(m_PushConstants[name].Type));
      glUniformMatrix2x4dv(m_PushConstants[name].Location, 1, false, glm::value_ptr(value));
   }


   void OpenGLPipeline::PushConstant(const std::string& name, const glm::dmat3x2& value) {
      PKZL_CORE_ASSERT(m_PushConstants[name].Type == DataType::DMat3x2, "Uniform '{0}' type mismatch.  DMat3x2 given, expected {1}!", name, DataTypeToString(m_PushConstants[name].Type));
      glUniformMatrix3x2dv(m_PushConstants[name].Location, 1, false, glm::value_ptr(value));
   }


   void OpenGLPipeline::PushConstant(const std::string& name, const glm::dmat3& value) {
      PKZL_CORE_ASSERT(m_PushConstants[name].Type == DataType::DMat3, "Uniform '{0}' type mismatch.  DMat3 given, expected {1}!", name, DataTypeToString(m_PushConstants[name].Type));
      glUniformMatrix3dv(m_PushConstants[name].Location, 1, false, glm::value_ptr(value));
   }


   void OpenGLPipeline::PushConstant(const std::string& name, const glm::dmat3x4& value) {
      PKZL_CORE_ASSERT(m_PushConstants[name].Type == DataType::DMat3x4, "Uniform '{0}' type mismatch.  DMat3x4 given, expected {1}!", name, DataTypeToString(m_PushConstants[name].Type));
      glUniformMatrix3x4dv(m_PushConstants[name].Location, 1, false, glm::value_ptr(value));
   }


   void OpenGLPipeline::PushConstant(const std::string& name, const glm::dmat4x2& value) {
      PKZL_CORE_ASSERT(m_PushConstants[name].Type == DataType::DMat4x2, "Uniform '{0}' type mismatch.  DMat4x2 given, expected {1}!", name, DataTypeToString(m_PushConstants[name].Type));
      glUniformMatrix4x2dv(m_PushConstants[name].Location, 1, false, glm::value_ptr(value));
   }


   void OpenGLPipeline::PushConstant(const std::string& name, const glm::dmat4x3& value) {
      PKZL_CORE_ASSERT(m_PushConstants[name].Type == DataType::DMat4x3, "Uniform '{0}' type mismatch.  DMat4x3 given, expected {1}!", name, DataTypeToString(m_PushConstants[name].Type));
      glUniformMatrix4x3dv(m_PushConstants[name].Location, 1, false, glm::value_ptr(value));
   }


   void OpenGLPipeline::PushConstant(const std::string& name, const glm::dmat4& value) {
      PKZL_CORE_ASSERT(m_PushConstants[name].Type == DataType::DMat4, "Uniform '{0}' type mismatch.  DMat4 given, expected {1}!", name, DataTypeToString(m_PushConstants[name].Type));
      glUniformMatrix4dv(m_PushConstants[name].Location, 1, GL_FALSE, glm::value_ptr(value));
   }


   void OpenGLPipeline::AppendShader(ShaderType type, const std::filesystem::path path) {
      PKZL_CORE_LOG_TRACE("Appending shader '{0}'", path.string());

      std::vector<uint32_t> src = ReadFile<uint32_t>(path);

      spirv_cross::CompilerGLSL compiler(src);
      m_PushConstants.merge(ParsePushConstants(compiler));

      std::string glsl = compiler.compile();
      m_ShaderIds.emplace_back(glCreateShader(ShaderTypeToOpenGLType(type)));
      m_ShaderSrcs.emplace_back(CompileGlslToSpv(RenderCore::API::OpenGL, type, "spirv-cross", glsl.data(), glsl.size()));

      GLuint shaderId = m_ShaderIds.back();
      std::vector<uint32_t>& srcOpenGL = m_ShaderSrcs.back();
      
      glShaderBinary(1, &shaderId, GL_SHADER_BINARY_FORMAT_SPIR_V, srcOpenGL.data(), static_cast<GLsizei>(srcOpenGL.size() * sizeof(uint32_t)));
      glSpecializeShader(shaderId, "main", 0, nullptr, nullptr);

      GLint isCompiled = 0;
      glGetShaderiv(shaderId, GL_COMPILE_STATUS, &isCompiled);
      if (isCompiled == GL_FALSE) {
         GLint maxLength = 0;
         glGetShaderiv(shaderId, GL_INFO_LOG_LENGTH, &maxLength);

         std::vector<GLchar> infoLog(maxLength);
         glGetShaderInfoLog(shaderId, maxLength, &maxLength, &infoLog[0]);

         glDeleteShader(shaderId);

         PKZL_CORE_LOG_ERROR("{0}", infoLog.data());
         throw std::runtime_error("Shader compilation failure!");
      }
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
         throw std::runtime_error("Shader link failure!");
      }

      for (const auto shaderId : m_ShaderIds) {
         glDetachShader(m_RendererId, shaderId);
      }
   }


   void OpenGLPipeline::ReflectShaders() {
      PKZL_CORE_LOG_TRACE("Reflecting shaders");
      glUseProgram(m_RendererId);

      for (auto& [name, uniform] : m_PushConstants) {
         uniform.Location = glGetUniformLocation(m_RendererId, name.data());
         PKZL_CORE_LOG_TRACE("Uniform '{0}' is at location {1}", name, uniform.Location);
      }

      for (const auto& src : m_ShaderSrcs) {
         spirv_cross::Compiler compiler(src);
         spirv_cross::ShaderResources resources = compiler.get_shader_resources();

         uint32_t bufferIndex = 0;
         for (const auto& resource : resources.uniform_buffers) {
            auto& type = compiler.get_type(resource.base_type_id);
            size_t memberCount = type.member_types.size();
            uint32_t bindingPoint = compiler.get_decoration(resource.id, spv::DecorationBinding);

            if (s_UniformBuffers.find(bindingPoint) == s_UniformBuffers.end()) {
               ShaderUniformBuffer& uniformBuffer = s_UniformBuffers[bindingPoint];
               uniformBuffer.Name = resource.name;
               uniformBuffer.BindingPoint = bindingPoint;
               uniformBuffer.Size = static_cast<uint32_t>(compiler.get_declared_struct_size(type));

               glCreateBuffers(1, &uniformBuffer.RendererId);
               glBindBuffer(GL_UNIFORM_BUFFER, uniformBuffer.RendererId);
               glBufferData(GL_UNIFORM_BUFFER, uniformBuffer.Size, nullptr, GL_DYNAMIC_DRAW);
               glBindBufferBase(GL_UNIFORM_BUFFER, uniformBuffer.BindingPoint, uniformBuffer.RendererId);
               PKZL_CORE_LOG_TRACE("Created Uniform Buffer at binding point {0} with name '{1}', size is {2} bytes", uniformBuffer.BindingPoint, uniformBuffer.Name, uniformBuffer.Size);
               glBindBuffer(GL_UNIFORM_BUFFER, 0);
            }
         }

         int32_t sampler = 0;
         for (const auto& resource : resources.sampled_images) {
            auto& type = compiler.get_type(resource.base_type_id);
            uint32_t bindingPoint = compiler.get_decoration(resource.id, spv::DecorationBinding);
            const auto& name = resource.name;
            uint32_t dimension = type.image.dim;

            GLint location = glGetUniformLocation(m_RendererId, name.c_str());
            PKZL_CORE_ASSERT(location != -1, "Could not find uniform location for {0}", name);
            m_Resources.emplace(std::make_pair(name, ShaderResourceDeclaration {name, bindingPoint,1}));
            glUniform1i(location, bindingPoint);
            PKZL_CORE_LOG_TRACE("Found image sampler at binding point {0} with name '{1}', dimension is {2}", bindingPoint, name, dimension);
         }
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
