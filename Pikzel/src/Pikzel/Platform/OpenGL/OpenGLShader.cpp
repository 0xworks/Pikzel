#include "glpch.h"
#include "OpenGLShader.h"

#include <glm/gtc/type_ptr.hpp>

namespace Pikzel {

   GLuint CreateShaderModule(const std::vector<char>& src, GLenum type) {
      GLuint shader = glCreateShader(type);
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


   GLuint CreateShaderProgram(GLuint vertexShader, GLuint fragmentShader) {
      GLuint program = glCreateProgram();

      glAttachShader(program, vertexShader);
      glAttachShader(program, fragmentShader);

      glLinkProgram(program);

      GLint isLinked = 0;
      glGetProgramiv(program, GL_LINK_STATUS, &isLinked);
      if (isLinked == GL_FALSE) {
         GLint maxLength = 0;
         glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);

         std::vector<GLchar> infoLog(maxLength);
         glGetProgramInfoLog(program, maxLength, &maxLength, &infoLog[0]);

         glDeleteProgram(program);
         glDeleteShader(vertexShader);
         glDeleteShader(fragmentShader);

         PKZL_CORE_LOG_ERROR("{0}", infoLog.data());
         throw std::runtime_error("Shader link failure!");
      }
      return program;
   }


   OpenGLShader::OpenGLShader(const std::vector<char>& vertexSrc, const std::vector<char>& fragmentSrc) {
      PKZL_PROFILE_FUNCTION();

      GLuint vertexShader = CreateShaderModule(vertexSrc, GL_VERTEX_SHADER);
      GLuint fragmentShader = CreateShaderModule(fragmentSrc, GL_FRAGMENT_SHADER);

      m_RendererID = CreateShaderProgram(vertexShader, fragmentShader);

      for (auto shader : {vertexShader, fragmentShader}) {
         glDetachShader(m_RendererID, shader);
         glDeleteShader(shader);
      }
   }


   OpenGLShader::~OpenGLShader() {
      PKZL_PROFILE_FUNCTION();
      glDeleteProgram(m_RendererID);
   }


   void OpenGLShader::Bind() const {
      PKZL_PROFILE_FUNCTION();
      glUseProgram(m_RendererID);
   }


   void OpenGLShader::Unbind() const {
      PKZL_PROFILE_FUNCTION();
      glUseProgram(0);
   }


   void OpenGLShader::SetInt(const std::string& name, int value) {
      PKZL_PROFILE_FUNCTION();
      UploadUniformInt(name, value);
   }


   void OpenGLShader::SetIntArray(const std::string& name, int* values, uint32_t count) {
      PKZL_PROFILE_FUNCTION();
      UploadUniformIntArray(name, values, count);
   }


   void OpenGLShader::SetFloat(const std::string& name, float value) {
      PKZL_PROFILE_FUNCTION();
      UploadUniformFloat(name, value);
   }


   void OpenGLShader::SetFloat3(const std::string& name, const glm::vec3& value) {
      PKZL_PROFILE_FUNCTION();
      UploadUniformFloat3(name, value);
   }


   void OpenGLShader::SetFloat4(const std::string& name, const glm::vec4& value) {
      PKZL_PROFILE_FUNCTION();
      UploadUniformFloat4(name, value);
   }


   void OpenGLShader::SetMat4(const std::string& name, const glm::mat4& value) {
      PKZL_PROFILE_FUNCTION();
      UploadUniformMat4(name, value);
   }


   void OpenGLShader::UploadUniformInt(const std::string& name, int value) {
      GLint location = glGetUniformLocation(m_RendererID, name.c_str());
      glUniform1i(location, value);
   }


   void OpenGLShader::UploadUniformIntArray(const std::string& name, int* values, uint32_t count) {
      GLint location = glGetUniformLocation(m_RendererID, name.c_str());
      glUniform1iv(location, count, values);
   }


   void OpenGLShader::UploadUniformFloat(const std::string& name, float value) {
      GLint location = glGetUniformLocation(m_RendererID, name.c_str());
      glUniform1f(location, value);
   }


   void OpenGLShader::UploadUniformFloat2(const std::string& name, const glm::vec2& value) {
      GLint location = glGetUniformLocation(m_RendererID, name.c_str());
      glUniform2f(location, value.x, value.y);
   }


   void OpenGLShader::UploadUniformFloat3(const std::string& name, const glm::vec3& value) {
      GLint location = glGetUniformLocation(m_RendererID, name.c_str());
      glUniform3f(location, value.x, value.y, value.z);
   }


   void OpenGLShader::UploadUniformFloat4(const std::string& name, const glm::vec4& value) {
      GLint location = glGetUniformLocation(m_RendererID, name.c_str());
      glUniform4f(location, value.x, value.y, value.z, value.w);
   }


   void OpenGLShader::UploadUniformMat3(const std::string& name, const glm::mat3& matrix) {
      GLint location = glGetUniformLocation(m_RendererID, name.c_str());
      glUniformMatrix3fv(location, 1, GL_FALSE, glm::value_ptr(matrix));
   }


   void OpenGLShader::UploadUniformMat4(const std::string& name, const glm::mat4& matrix) {
      GLint location = glGetUniformLocation(m_RendererID, name.c_str());
      glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(matrix));
   }

}
