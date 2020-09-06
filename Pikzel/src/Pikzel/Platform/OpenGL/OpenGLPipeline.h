#pragma once

#include "Pikzel/Renderer/GraphicsContext.h"
#include "Pikzel/Renderer/Pipeline.h"

#include <glm/glm.hpp>
#include <vector>

namespace Pikzel {


   class OpenGLPipeline : public Pipeline {
   public:
      OpenGLPipeline(GraphicsContext& gc, const PipelineSettings& settings);
      virtual ~OpenGLPipeline();

      virtual void SetInt(const std::string& name, int value) override;
      virtual void SetIntArray(const std::string& name, int* values, uint32_t count) override;
      virtual void SetFloat(const std::string& name, float value) override;
      virtual void SetFloat3(const std::string& name, const glm::vec3& value) override;
      virtual void SetFloat4(const std::string& name, const glm::vec4& value) override;
      virtual void SetMat4(const std::string& name, const glm::mat4& value) override;

      GLuint GetRendererId() const;
      GLuint GetVAORendererId() const;

   private:
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
      uint32_t m_VAORendererID;

   };

}
