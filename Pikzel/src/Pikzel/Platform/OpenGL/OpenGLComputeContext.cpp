#include "OpenGLComputeContext.h"

#include "OpenGLBuffer.h"
#include "OpenGLPipeline.h"
#include "OpenGLTexture.h"

#include <GL/gl.h>

#include <format>
#include <memory>
#include <stdexcept>

namespace Pikzel {

   OpenGLComputeContext::OpenGLComputeContext()
   : m_Pipeline {nullptr}
   {}


   void OpenGLComputeContext::Begin() {}


   void OpenGLComputeContext::End() {}


   void OpenGLComputeContext::Bind(const Id resourceId, const UniformBuffer& buffer) {
      glBindBufferBase(GL_UNIFORM_BUFFER, m_Pipeline->GetUniformBufferBinding(resourceId), static_cast<const OpenGLUniformBuffer&>(buffer).GetRendererId());
   }


   void OpenGLComputeContext::Unbind(const UniformBuffer&) {}


   void OpenGLComputeContext::Bind(const Id resourceId, const Texture& texture, const uint32_t mipLevel) {
      GLuint samplerBinding = m_Pipeline->GetSamplerBinding(resourceId, false);
      if (samplerBinding != ~0) {
         glBindTextureUnit(samplerBinding, static_cast<const OpenGLTexture&>(texture).GetRendererId());
      } else {
         GLuint storageImageBinding = m_Pipeline->GetStorageImageBinding(resourceId, false);
         if (storageImageBinding != ~0) {
            glBindImageTexture(storageImageBinding, static_cast<const OpenGLTexture&>(texture).GetRendererId(), mipLevel == ~0? 0 : mipLevel, GL_TRUE, 0, GL_WRITE_ONLY, TextureFormatToInternalFormat(texture.GetFormat()));
         } else {
            throw std::invalid_argument {std::format("OpenGLComputeContext::Bind(const Texture&) failed to find binding with id {}!", resourceId)};
         }
      }
   }


   void OpenGLComputeContext::Unbind(const Texture&) {}


   void OpenGLComputeContext::Bind(const Pipeline& pipeline) {
      const OpenGLPipeline& glPipeline = static_cast<const OpenGLPipeline&>(pipeline);
      glPipeline.SetGLState();
      m_Pipeline = const_cast<OpenGLPipeline*>(&glPipeline);
   }


   void OpenGLComputeContext::Unbind(const Pipeline& pipeline) {
      m_Pipeline = nullptr;
      glBindVertexArray(0);
      glUseProgram(0);
   }


   std::unique_ptr<Pikzel::Pipeline> OpenGLComputeContext::CreatePipeline(const PipelineSettings& settings) {
      return std::make_unique<OpenGLPipeline>(settings);
   }


   void OpenGLComputeContext::PushConstant(const Id id, bool value) {
      PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
      m_Pipeline->PushConstant(id, value);
   }


   void OpenGLComputeContext::PushConstant(const Id id, int value) {
      PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
      m_Pipeline->PushConstant(id, value);
   }


   void OpenGLComputeContext::PushConstant(const Id id, uint32_t value) {
      PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
      m_Pipeline->PushConstant(id, value);
   }


   void OpenGLComputeContext::PushConstant(const Id id, float value) {
      PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
      m_Pipeline->PushConstant(id, value);
   }


   void OpenGLComputeContext::PushConstant(const Id id, double value) {
      PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
      m_Pipeline->PushConstant(id, value);
   }


   void OpenGLComputeContext::PushConstant(const Id id, const glm::bvec2& value) {
      PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
      m_Pipeline->PushConstant(id, value);
   }

   
   void OpenGLComputeContext::PushConstant(const Id id, const glm::bvec3& value) {
      PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
      m_Pipeline->PushConstant(id, value);
   }


   void OpenGLComputeContext::PushConstant(const Id id, const glm::bvec4& value) {
      PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
      m_Pipeline->PushConstant(id, value);
   }


   void OpenGLComputeContext::PushConstant(const Id id, const glm::ivec2& value) {
      PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
      m_Pipeline->PushConstant(id, value);
   }


   void OpenGLComputeContext::PushConstant(const Id id, const glm::ivec3& value) {
      PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
      m_Pipeline->PushConstant(id, value);
   }


   void OpenGLComputeContext::PushConstant(const Id id, const glm::ivec4& value) {
      PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
      m_Pipeline->PushConstant(id, value);
   }


   void OpenGLComputeContext::PushConstant(const Id id, const glm::uvec2& value) {
      PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
      m_Pipeline->PushConstant(id, value);
   }


   void OpenGLComputeContext::PushConstant(const Id id, const glm::uvec3& value) {
      PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
      m_Pipeline->PushConstant(id, value);
   }


   void OpenGLComputeContext::PushConstant(const Id id, const glm::uvec4& value) {
      PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
      m_Pipeline->PushConstant(id, value);
   }


   void OpenGLComputeContext::PushConstant(const Id id, const glm::vec2& value) {
      PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
      m_Pipeline->PushConstant(id, value);
   }


   void OpenGLComputeContext::PushConstant(const Id id, const glm::vec3& value) {
      PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
      m_Pipeline->PushConstant(id, value);
   }


   void OpenGLComputeContext::PushConstant(const Id id, const glm::vec4& value) {
      PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
      m_Pipeline->PushConstant(id, value);
   }


   void OpenGLComputeContext::PushConstant(const Id id, const glm::dvec2& value) {
      PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
      m_Pipeline->PushConstant(id, value);
   }


   void OpenGLComputeContext::PushConstant(const Id id, const glm::dvec3& value) {
      PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
      m_Pipeline->PushConstant(id, value);
   }


   void OpenGLComputeContext::PushConstant(const Id id, const glm::dvec4& value) {
      PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
      m_Pipeline->PushConstant(id, value);
   }


   void OpenGLComputeContext::PushConstant(const Id id, const glm::mat2& value) {
      PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
      m_Pipeline->PushConstant(id, value);
   }


   //void OpenGLComputeContext::PushConstant(const Id id, const glm::mat2x3& value) {
   //   PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
   //   m_Pipeline->PushConstant(id, value);
   //}


   void OpenGLComputeContext::PushConstant(const Id id, const glm::mat2x4& value) {
      PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
      m_Pipeline->PushConstant(id, value);
   }


   void OpenGLComputeContext::PushConstant(const Id id, const glm::mat3x2& value) {
      PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
      m_Pipeline->PushConstant(id, value);
   }


   //void OpenGLComputeContext::PushConstant(const Id id, const glm::mat3& value) {
   //   PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
   //   m_Pipeline->PushConstant(id, value);
   //}


   void OpenGLComputeContext::PushConstant(const Id id, const glm::mat3x4& value) {
      PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
      m_Pipeline->PushConstant(id, value);
   }


   void OpenGLComputeContext::PushConstant(const Id id, const glm::mat4x2& value) {
      PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
      m_Pipeline->PushConstant(id, value);
   }


   //void OpenGLComputeContext::PushConstant(const Id id, const glm::mat4x3& value) {
   //   PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
   //   m_Pipeline->PushConstant(id, value);
   //}


   void OpenGLComputeContext::PushConstant(const Id id, const glm::mat4& value) {
      PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
      m_Pipeline->PushConstant(id, value);
   }


   void OpenGLComputeContext::PushConstant(const Id id, const glm::dmat2& value) {
      PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
      m_Pipeline->PushConstant(id, value);
   }


   //void OpenGLComputeContext::PushConstant(const Id id, const glm::dmat2x3& value) {
   //   PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
   //   m_Pipeline->PushConstant(id, value);
   //}


   void OpenGLComputeContext::PushConstant(const Id id, const glm::dmat2x4& value) {
      PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
      m_Pipeline->PushConstant(id, value);
   }


   void OpenGLComputeContext::PushConstant(const Id id, const glm::dmat3x2& value) {
      PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
      m_Pipeline->PushConstant(id, value);
   }


   //void OpenGLComputeContext::PushConstant(const Id id, const glm::dmat3& value) {
   //   PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
   //   m_Pipeline->PushConstant(id, value);
   //}


   void OpenGLComputeContext::PushConstant(const Id id, const glm::dmat3x4& value) {
      PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
      m_Pipeline->PushConstant(id, value);
   }


   void OpenGLComputeContext::PushConstant(const Id id, const glm::dmat4x2& value) {
      PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
      m_Pipeline->PushConstant(id, value);
   }


   //void OpenGLComputeContext::PushConstant(const Id id, const glm::dmat4x3& value) {
   //   PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
   //   m_Pipeline->PushConstant(id, value);
   //}


   void OpenGLComputeContext::PushConstant(const Id id, const glm::dmat4& value) {
      PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
      m_Pipeline->PushConstant(id, value);
   }


   void OpenGLComputeContext::Dispatch(const uint32_t x, const uint32_t y, const uint32_t z) {
      PKZL_PROFILE_FUNCTION();
      glDispatchCompute(x, y, z);
   }

}
