#pragma once

#include "VulkanDevice.h"
#include "VulkanGraphicsContext.h"

#include "Pikzel/Renderer/Pipeline.h"

#include <filesystem>
#include <unordered_map>

namespace Pikzel {

   class Window;

   struct ShaderPushConstant {
      std::string Name;
      DataType Type = DataType::None;
      vk::ShaderStageFlags ShaderStages = {};
      uint32_t Offset = 0;
      uint32_t Size = 0;
   };


   class VulkanPipeline : public Pipeline {
   public:
      VulkanPipeline(std::shared_ptr<VulkanDevice> device, VulkanGraphicsContext& gc, const PipelineSettings& settings);
      virtual ~VulkanPipeline();

   public:
      vk::Pipeline GetVkPipeline() const;
      vk::PipelineLayout GetVkPipelineLayout() const;

      const ShaderPushConstant& GetPushConstant(const std::string& name) const;

   private:
      vk::ShaderModule CreateShaderModule(ShaderType type, const std::vector<uint32_t>& src);
      void DestroyShaderModule(vk::ShaderModule& shaderModule);

      void CreateDescriptorSetLayout(const PipelineSettings& settings);
      void DestroyDescriptorSetLayout();

      void CreatePipelineLayout(const PipelineSettings& settings);
      void DestroyPipelineLayout();

      void CreatePipeline(const VulkanGraphicsContext& gc, const PipelineSettings& settings);
      void DestroyPipeline();

   private:
      std::shared_ptr<VulkanDevice> m_Device;
      vk::DescriptorSetLayout m_DescriptorSetLayout;
      vk::PipelineLayout m_PipelineLayout;
      vk::Pipeline m_Pipeline;
      std::vector<std::pair<ShaderType, std::vector<uint32_t>>> m_ShaderSrcs;
      std::unordered_map<std::string, ShaderPushConstant> m_PushConstants;
   };

}
