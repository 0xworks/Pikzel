#pragma once

#include "Pikzel/Renderer/Pipeline.h"
#include "VulkanDevice.h"

namespace Pikzel {

   class Window;

   class VulkanPipeline : public Pipeline {
   public:
      VulkanPipeline(std::shared_ptr<VulkanDevice> device, const Window& window, const PipelineSettings& settings);
      virtual ~VulkanPipeline();

      void Bind() const override;
      void Unbind() const override;

      void SetInt(const std::string& name, int value) override;
      void SetIntArray(const std::string& name, int* values, uint32_t count) override;
      void SetFloat(const std::string& name, float value) override;
      void SetFloat3(const std::string& name, const glm::vec3& value) override;
      void SetFloat4(const std::string& name, const glm::vec4& value) override;
      void SetMat4(const std::string& name, const glm::mat4& value) override;

   private:
      void CreateDescriptorSetLayout();
      void DestroyDescriptorSetLayout();

      void CreatePipelineLayout();
      void DestroyPipelineLayout();

      void CreatePipeline(const Window& window, const PipelineSettings& settings);
      void DestroyPipeline();

   private:
      std::shared_ptr<VulkanDevice> m_Device;
      vk::DescriptorSetLayout m_DescriptorSetLayout;
      vk::PipelineLayout m_PipelineLayout;
      vk::Pipeline m_Pipeline;
   };

}
