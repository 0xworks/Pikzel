#pragma once

#include "Pikzel/Renderer/Shader.h"
#include <glm/glm.hpp>
#include <vector>

namespace Pikzel {

   class OpenGLShader : public Shader {
   public:
      OpenGLShader(const std::vector<char>& vertexSrc, const std::vector<char>& fragmentSrc);
      virtual ~OpenGLShader();

      virtual void Bind() const override;
      virtual void Unbind() const override;

      virtual void SetInt(const std::string& name, int value) override;
      virtual void SetIntArray(const std::string& name, int* values, uint32_t count) override;
      virtual void SetFloat(const std::string& name, float value) override;
      virtual void SetFloat3(const std::string& name, const glm::vec3& value) override;
      virtual void SetFloat4(const std::string& name, const glm::vec4& value) override;
      virtual void SetMat4(const std::string& name, const glm::mat4& value) override;

      void UploadUniformInt(const std::string& name, int value);
      void UploadUniformIntArray(const std::string& name, int* values, uint32_t count);

      void UploadUniformFloat(const std::string& name, float value);
      void UploadUniformFloat2(const std::string& name, const glm::vec2& value);
      void UploadUniformFloat3(const std::string& name, const glm::vec3& value);
      void UploadUniformFloat4(const std::string& name, const glm::vec4& value);

      void UploadUniformMat3(const std::string& name, const glm::mat3& matrix);
      void UploadUniformMat4(const std::string& name, const glm::mat4& matrix);

   private:
      uint32_t m_RendererID;
   };

}
