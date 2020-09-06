#pragma once

#include "VulkanDevice.h"
#include "VulkanGraphicsContext.h"

#include "Pikzel/Renderer/Pipeline.h"

#include <filesystem>

namespace Pikzel {

   class Window;

   class VulkanPipeline : public Pipeline {
   public:
      VulkanPipeline(std::shared_ptr<VulkanDevice> device, VulkanGraphicsContext& gc, const PipelineSettings& settings);
      virtual ~VulkanPipeline();

      void SetInt(const std::string& name, int value) override;
      void SetIntArray(const std::string& name, int* values, uint32_t count) override;
      void SetFloat(const std::string& name, float value) override;
      void SetFloat3(const std::string& name, const glm::vec3& value) override;
      void SetFloat4(const std::string& name, const glm::vec4& value) override;
      void SetMat4(const std::string& name, const glm::mat4& value) override;

   public:
      vk::Pipeline GetVkPipeline() const;

   private:
      vk::ShaderModule CreateShaderModule(ShaderType type, const std::filesystem::path path);
      void DestroyShaderModule(vk::ShaderModule& shaderModule);

      void CreateDescriptorSetLayout();
      void DestroyDescriptorSetLayout();

      void CreatePipelineLayout();
      void DestroyPipelineLayout();

      void CreatePipeline(const VulkanGraphicsContext& gc, const PipelineSettings& settings);
      void DestroyPipeline();

   private:
      std::shared_ptr<VulkanDevice> m_Device;
      vk::DescriptorSetLayout m_DescriptorSetLayout;
      vk::PipelineLayout m_PipelineLayout;
      vk::Pipeline m_Pipeline;
   };

}
