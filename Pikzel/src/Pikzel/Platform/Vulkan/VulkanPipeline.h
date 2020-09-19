#pragma once

#include "VulkanDevice.h"
#include "VulkanGraphicsContext.h"

#include "Pikzel/Renderer/Pipeline.h"

#include <filesystem>
#include <unordered_map>

namespace Pikzel {

   class Window;

   struct VulkanPushConstant {
      std::string Name;
      DataType Type = DataType::None;
      vk::ShaderStageFlags ShaderStages = {};
      uint32_t Offset = 0;
      uint32_t Size = 0;
   };


   struct VulkanResource {
      std::string Name;
      uint32_t DescriptorSet = 0;
      uint32_t Binding = 0;
      vk::DescriptorType Type = {};
      uint32_t Count = 0;
      vk::ShaderStageFlags ShaderStages = {};
   };


   class VulkanPipeline : public Pipeline {
   public:
      VulkanPipeline(std::shared_ptr<VulkanDevice> device, VulkanGraphicsContext& gc, const PipelineSettings& settings);
      virtual ~VulkanPipeline();

   public:
      vk::Pipeline GetVkPipeline() const;
      vk::PipelineLayout GetVkPipelineLayout() const;
      const std::vector<vk::DescriptorSet>& GetVkDescriptorSets(const uint32_t i) const;

      const VulkanPushConstant& GetPushConstant(const entt::id_type id) const;
      const VulkanResource& GetResource(const entt::id_type id) const;


   private:

      vk::ShaderModule CreateShaderModule(ShaderType type, const std::vector<uint32_t>& src);
      void DestroyShaderModule(vk::ShaderModule& shaderModule);

      void ReflectShaders();

      void CreateDescriptorSetLayout(const PipelineSettings& settings);
      void DestroyDescriptorSetLayout();

      void CreatePipelineLayout();
      void DestroyPipelineLayout();

      void CreatePipeline(const VulkanGraphicsContext& gc, const PipelineSettings& settings);
      void DestroyPipeline();

      void CreateDescriptorPool();
      void DestroyDesciptorPool();

      void CreateDescriptorSets();
      void DestroyDescriptorSets();

   private:
      std::shared_ptr<VulkanDevice> m_Device;
      std::vector<vk::DescriptorSetLayout> m_DescriptorSetLayouts;
      vk::PipelineLayout m_PipelineLayout;
      vk::Pipeline m_Pipeline;
      vk::DescriptorPool m_DescriptorPool;
      std::vector<std::vector<vk::DescriptorSet>> m_DescriptorSets;
      std::vector<std::pair<ShaderType, std::vector<uint32_t>>> m_ShaderSrcs;
      std::unordered_map<entt::id_type, VulkanPushConstant> m_PushConstants;
      std::unordered_map<entt::id_type, VulkanResource> m_Resources;
   };

}
