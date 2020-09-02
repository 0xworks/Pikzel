#include "glpch.h"
#include "OpenGLPipeline.h"
#include "OpenGLBuffer.h"

#include <glm/gtc/type_ptr.hpp>

namespace Pikzel {

   static GLenum DataTypeToOpenGLType(DataType type) {
      switch (type) {
         case DataType::Float:    return GL_FLOAT;
         case DataType::Float2:   return GL_FLOAT;
         case DataType::Float3:   return GL_FLOAT;
         case DataType::Float4:   return GL_FLOAT;
         case DataType::Mat3:     return GL_FLOAT;
         case DataType::Mat4:     return GL_FLOAT;
         case DataType::Int:      return GL_INT;
         case DataType::Int2:     return GL_INT;
         case DataType::Int3:     return GL_INT;
         case DataType::Int4:     return GL_INT;
         case DataType::Bool:     return GL_BOOL;
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


   static GLuint CreateShaderModule(ShaderType type, const std::vector<char>& src) {

      GLuint shader = glCreateShader(ShaderTypeToOpenGLType(type));
      const GLchar* srcC = src.data();
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
         throw std::runtime_error("Shader compilation failure!");
      }
      return shader;
   }


   static GLuint CreateShaderProgram(const std::vector<GLuint>& shaders) {
      GLuint program = glCreateProgram();

      for (const auto shader : shaders) {
         glAttachShader(program, shader);
      }

      glLinkProgram(program);

      GLint isLinked = 0;
      glGetProgramiv(program, GL_LINK_STATUS, &isLinked);
      if (isLinked == GL_FALSE) {
         GLint maxLength = 0;
         glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);

         std::vector<GLchar> infoLog(maxLength);
         glGetProgramInfoLog(program, maxLength, &maxLength, &infoLog[0]);

         glDeleteProgram(program);
         for (const auto shader : shaders) {
            glDeleteShader(shader);
         }

         PKZL_CORE_LOG_ERROR("{0}", infoLog.data());
         throw std::runtime_error("Shader link failure!");
      }

      for (const auto shader : shaders) {
         glDetachShader(program, shader);
      }

      return program;
   }


   OpenGLPipeline::OpenGLPipeline(const PipelineSettings& settings) {
      PKZL_PROFILE_FUNCTION();

      std::vector<GLuint> shaders;
      for (const auto& [shaderType, src] : settings.Shaders) {
         shaders.emplace_back(CreateShaderModule(shaderType, src));
      }

      m_RendererID = CreateShaderProgram(shaders);

      for (auto shader : shaders) {
         glDeleteShader(shader);
      }
      glCreateVertexArrays(1, &m_VAORendererID);
      glBindVertexArray(m_VAORendererID);
      settings.VertexBuffer.Bind();

      GLuint vertexAttributeIndex = 0;
      for (const auto& element : settings.VertexBuffer.GetLayout()) {
         switch (element.Type) {
            case DataType::Float:
            case DataType::Float2:
            case DataType::Float3:
            case DataType::Float4:
            case DataType::Int:
            case DataType::Int2:
            case DataType::Int3:
            case DataType::Int4:
            case DataType::Bool:
            {
               glEnableVertexAttribArray(vertexAttributeIndex);
               glVertexAttribPointer(vertexAttributeIndex, element.GetComponentCount(), DataTypeToOpenGLType(element.Type), element.Normalized ? GL_TRUE : GL_FALSE, settings.VertexBuffer.GetLayout().GetStride(), (const void*)element.Offset);
               ++vertexAttributeIndex;
               break;
            }
            case DataType::Mat3:
            case DataType::Mat4:
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
      PKZL_PROFILE_FUNCTION();
      glDeleteVertexArrays(1, &m_VAORendererID);
      glDeleteProgram(m_RendererID);
   }


   void OpenGLPipeline::Bind() const {
      PKZL_PROFILE_FUNCTION();
      glUseProgram(m_RendererID);
      glBindVertexArray(m_VAORendererID);
   }


   void OpenGLPipeline::Unbind() const {
      PKZL_PROFILE_FUNCTION();
      glBindVertexArray(0);
      glUseProgram(0);
   }


   void OpenGLPipeline::SetInt(const std::string& name, int value) {
      PKZL_PROFILE_FUNCTION();
      UploadUniformInt(name, value);
   }


   void OpenGLPipeline::SetIntArray(const std::string& name, int* values, uint32_t count) {
      PKZL_PROFILE_FUNCTION();
      UploadUniformIntArray(name, values, count);
   }


   void OpenGLPipeline::SetFloat(const std::string& name, float value) {
      PKZL_PROFILE_FUNCTION();
      UploadUniformFloat(name, value);
   }


   void OpenGLPipeline::SetFloat3(const std::string& name, const glm::vec3& value) {
      PKZL_PROFILE_FUNCTION();
      UploadUniformFloat3(name, value);
   }


   void OpenGLPipeline::SetFloat4(const std::string& name, const glm::vec4& value) {
      PKZL_PROFILE_FUNCTION();
      UploadUniformFloat4(name, value);
   }


   void OpenGLPipeline::SetMat4(const std::string& name, const glm::mat4& value) {
      PKZL_PROFILE_FUNCTION();
      UploadUniformMat4(name, value);
   }


   void OpenGLPipeline::UploadUniformInt(const std::string& name, int value) {
      GLint location = glGetUniformLocation(m_RendererID, name.c_str());
      glUniform1i(location, value);
   }


   void OpenGLPipeline::UploadUniformIntArray(const std::string& name, int* values, uint32_t count) {
      GLint location = glGetUniformLocation(m_RendererID, name.c_str());
      glUniform1iv(location, count, values);
   }


   void OpenGLPipeline::UploadUniformFloat(const std::string& name, float value) {
      GLint location = glGetUniformLocation(m_RendererID, name.c_str());
      glUniform1f(location, value);
   }


   void OpenGLPipeline::UploadUniformFloat2(const std::string& name, const glm::vec2& value) {
      GLint location = glGetUniformLocation(m_RendererID, name.c_str());
      glUniform2f(location, value.x, value.y);
   }


   void OpenGLPipeline::UploadUniformFloat3(const std::string& name, const glm::vec3& value) {
      GLint location = glGetUniformLocation(m_RendererID, name.c_str());
      glUniform3f(location, value.x, value.y, value.z);
   }


   void OpenGLPipeline::UploadUniformFloat4(const std::string& name, const glm::vec4& value) {
      GLint location = glGetUniformLocation(m_RendererID, name.c_str());
      glUniform4f(location, value.x, value.y, value.z, value.w);
   }


   void OpenGLPipeline::UploadUniformMat3(const std::string& name, const glm::mat3& matrix) {
      GLint location = glGetUniformLocation(m_RendererID, name.c_str());
      glUniformMatrix3fv(location, 1, GL_FALSE, glm::value_ptr(matrix));
   }


   void OpenGLPipeline::UploadUniformMat4(const std::string& name, const glm::mat4& matrix) {
      GLint location = glGetUniformLocation(m_RendererID, name.c_str());
      glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(matrix));
   }

}
