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
      std::vector<uint32_t> Shape = {};
      vk::ShaderStageFlags ShaderStages = {};

      uint32_t GetCount() const {
         uint32_t count = 1;
         for (const auto dimension : Shape) {
            count *= dimension;
         }
         return count;
      }
   };


   class VulkanPipeline : public Pipeline {
   public:
      // construct compute pipeline with settings
      VulkanPipeline(std::shared_ptr<VulkanDevice> device, const PipelineSettings& settings);

      // construct graphics pipeline with context and settings
      VulkanPipeline(std::shared_ptr<VulkanDevice> device, const VulkanGraphicsContext& gc, const PipelineSettings& settings);

      virtual ~VulkanPipeline();

   public:

      std::shared_ptr<VulkanDevice> GetDevice();

      const std::vector<vk::DescriptorSetLayout>& GetVkDescriptorSetLayouts() const;
      vk::DescriptorSet GetVkDescriptorSet(const uint32_t set);

      // Bind the descriptors into the specified commandbuffer.  They should be considered "in use" (i.e. do not change them)
      // until the specified fence is signaled.
      void BindDescriptorSets(vk::CommandBuffer commandBuffer, std::shared_ptr<VulkanFence> fence);

      // mark descriptors as not-bound (they might still be in use in some previously submitted frame (check fences)
      void UnbindDescriptorSets();

      vk::Pipeline GetVkPipelineCompute() const;

      // TODO: tidy this. It would be better to have just one GetVkPipeline()
      //       but need extended dynamic state for that...
      vk::Pipeline GetVkPipelineFrontFaceCCW() const;
      vk::Pipeline GetVkPipelineFrontFaceCW() const;

      vk::PipelineLayout GetVkPipelineLayout() const;

      const VulkanPushConstant& GetPushConstant(const Id id) const;
      const VulkanResource& GetResource(const Id id) const;

   private:

      vk::ShaderModule CreateShaderModule(ShaderType type, const std::vector<uint32_t>& src);
      void DestroyShaderModule(vk::ShaderModule& shaderModule);

      void ReflectShaders(const SpecializationConstantsMap& specializationConstants);

      void CreateDescriptorSetLayouts(const PipelineSettings& settings);
      void DestroyDescriptorSetLayouts();

      void CreatePipelineLayout();
      void DestroyPipelineLayout();

      void CreateComputePipeline(const PipelineSettings& settings);
      void CreateGraphicsPipeline(const VulkanGraphicsContext& gc, const PipelineSettings& settings);
      void DestroyPipeline();

      void CreateDescriptorPool();
      void DestroyDesciptorPool();

      vk::DescriptorSet AllocateDescriptorSet(const uint32_t set);

   private:
      std::shared_ptr<VulkanDevice> m_Device;
      std::vector<vk::DescriptorSetLayout> m_DescriptorSetLayouts;
      vk::PipelineBindPoint m_PipelineBindPoint;
      vk::PipelineLayout m_PipelineLayout;
      vk::Pipeline m_PipelineCompute;
      vk::Pipeline m_PipelineFrontFaceCCW; // }- Need to create two variations of graphics pipelines, one has front faces CCW and the other has them CW
      vk::Pipeline m_PipelineFrontFaceCW;  // }  If/when VK_EXT_extended_dynamic_state becomes more widely available (e.g. in the nvidia general release drivers)
                                           // }  then the front face winding order can be a dynamic state
      vk::DescriptorPool m_DescriptorPool;
      std::vector<std::vector<vk::DescriptorSet>> m_DescriptorSetInstances;         // m_DescriptorSets[i] = collection of descriptor sets that have been allocated for set i
      std::vector<std::vector<bool>> m_DescriptorSetBound;                          // m_DescriptorSetBound[i] = collection of booleans indicating which elements from m_DescriptorSets[i] are currently bound to the pipeline
      std::vector<std::vector<std::shared_ptr<VulkanFence>>> m_DescriptorSetFences; // m_DescriptorSetFences[i] = collection of fences synchronizing access to m_DescriptorSets for set i
      std::vector<uint32_t> m_DescriptorSetIndices;                                 // m_DescriptorSetIndices[i] = which element (of m_DescriptorSets) is currently available for writing for set i
      std::vector<bool> m_DescriptorSetPending;                                     // m_DescriptorSetPending[i] = true <=> set i needs to be bound for next draw call

      std::vector<std::pair<ShaderType, std::vector<uint32_t>>> m_ShaderSrcs;
      std::vector<vk::SpecializationInfo> m_ShaderSpecializations;
      std::vector<std::vector<vk::SpecializationMapEntry>> m_SpecializationMap;
      std::vector<std::vector<int32_t>> m_SpecializationData;
      std::unordered_map<Id, VulkanPushConstant> m_PushConstants;
      std::unordered_map<Id, VulkanResource> m_Resources;
   };

}
