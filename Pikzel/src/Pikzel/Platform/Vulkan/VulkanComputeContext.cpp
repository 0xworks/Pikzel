#include "VulkanComputeContext.h"

#include "VulkanBuffer.h"
#include "VulkanPipeline.h"
#include "VulkanTexture.h"
#include "VulkanUtility.h"

namespace Pikzel {

   VulkanComputeContext::VulkanComputeContext(std::shared_ptr<VulkanDevice> device)
   : m_Device {device}
   {
      CreateCommandPool();
      CreateCommandBuffers(1);
      CreateSyncObjects();
      CreatePipelineCache();
   }


   VulkanComputeContext::~VulkanComputeContext() {
      if (m_Device) {
         m_Device->GetVkDevice().waitIdle();
         if (m_Pipeline) {
            Unbind(*m_Pipeline);
         }
         DestroyPipelineCache();
         DestroySyncObjects();
         DestroyCommandBuffers();
         DestroyCommandPool();
      }
   }


   void VulkanComputeContext::Begin() {
      auto result = m_Device->GetVkDevice().waitForFences(GetFence()->GetVkFence(), true, UINT64_MAX);
      GetVkCommandBuffer().begin({vk::CommandBufferUsageFlagBits::eSimultaneousUse});
   }


   void VulkanComputeContext::End() {
      GetVkCommandBuffer().end();

      vk::SubmitInfo si;
      si.commandBufferCount = 1;
      si.pCommandBuffers = m_CommandBuffers.data();
      m_Device->GetVkDevice().resetFences(GetFence()->GetVkFence());
      m_Device->GetComputeQueue().submit(si, GetFence()->GetVkFence());
   }


   void VulkanComputeContext::Bind(const entt::id_type resourceId, const UniformBuffer& buffer) {
      const VulkanResource& resource = m_Pipeline->GetResource(resourceId);

      vk::DescriptorBufferInfo uniformBufferDescriptor = {
         static_cast<const VulkanUniformBuffer&>(buffer).GetVkBuffer() /*buffer*/,
         0                                                             /*offset*/,
         VK_WHOLE_SIZE                                                 /*range*/
      };

      vk::WriteDescriptorSet uniformBufferWrite = {
         m_Pipeline->GetVkDescriptorSet(resource.DescriptorSet)  /*dstSet*/,
         resource.Binding                                        /*dstBinding*/,
         0                                                       /*dstArrayElement*/,
         resource.GetCount()                                     /*descriptorCount*/,
         resource.Type                                           /*descriptorType*/,
         nullptr                                                 /*pImageInfo*/,
         &uniformBufferDescriptor                                /*pBufferInfo*/,
         nullptr                                                 /*pTexelBufferView*/
      };

      m_Device->GetVkDevice().updateDescriptorSets(uniformBufferWrite, nullptr);
   }


   void VulkanComputeContext::Unbind(const UniformBuffer&) {}


   void VulkanComputeContext::Bind(const entt::id_type resourceId, const Texture& texture, const uint32_t mipLevel) {
      const VulkanResource& resource = m_Pipeline->GetResource(resourceId);

      vk::Sampler sampler = static_cast<const VulkanTexture&>(texture).GetVkSampler();
      vk::ImageView imageView = static_cast<const VulkanTexture&>(texture).GetVkImageView(mipLevel);
      vk::DescriptorImageInfo textureImageDescriptor = {
         sampler,
         imageView,
         resource.Type == vk::DescriptorType::eStorageImage? vk::ImageLayout::eGeneral : vk::ImageLayout::eShaderReadOnlyOptimal
      };

      vk::WriteDescriptorSet textureSamplersWrite = {
         m_Pipeline->GetVkDescriptorSet(resource.DescriptorSet)  /*dstSet*/,
         resource.Binding                                        /*dstBinding*/,
         0                                                       /*dstArrayElement*/,
         resource.GetCount()                                     /*descriptorCount*/,
         resource.Type                                           /*descriptorType*/,
         &textureImageDescriptor                                 /*pImageInfo*/,
         nullptr                                                 /*pBufferInfo*/,
         nullptr                                                 /*pTexelBufferView*/
      };

      m_Device->GetVkDevice().updateDescriptorSets(textureSamplersWrite, nullptr);
   }


   void VulkanComputeContext::Unbind(const Texture&) {}


   void VulkanComputeContext::Bind(const Pipeline& pipeline) {
      const VulkanPipeline& vulkanPipeline = static_cast<const VulkanPipeline&>(pipeline);
      GetVkCommandBuffer().bindPipeline(vk::PipelineBindPoint::eCompute, vulkanPipeline.GetVkPipelineCompute());
      m_Pipeline = const_cast<VulkanPipeline*>(&vulkanPipeline);
      m_Pipeline->UnbindDescriptorSets(); // *un*bind descriptor sets here.  This allows us to update the descriptors.  The descriptor sets are then bound just before we Dispatch()
   }


   void VulkanComputeContext::Unbind(const Pipeline&) {
      m_Pipeline = nullptr;
   }


   std::unique_ptr<Pikzel::Pipeline> VulkanComputeContext::CreatePipeline(const PipelineSettings& settings) {
      return std::make_unique<VulkanPipeline>(m_Device, settings);
   }


   void VulkanComputeContext::PushConstant(const entt::id_type id, bool value) {
      PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
      const VulkanPushConstant& constant = m_Pipeline->GetPushConstant(id);
      PKZL_CORE_ASSERT(constant.Type == DataType::Bool, "Push constant '{0}' type mismatch.  Bool given, expected {1}!", constant.Name, DataTypeToString(constant.Type));
      GetVkCommandBuffer().pushConstants<bool>(m_Pipeline->GetVkPipelineLayout(), constant.ShaderStages, constant.Offset, value);
   }


   void VulkanComputeContext::PushConstant(const entt::id_type id, int value) {
      PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
      const VulkanPushConstant& constant = m_Pipeline->GetPushConstant(id);
      PKZL_CORE_ASSERT(constant.Type == DataType::Int, "Push constant '{0}' type mismatch.  Int given, expected {1}!", constant.Name, DataTypeToString(constant.Type));
      GetVkCommandBuffer().pushConstants<int>(m_Pipeline->GetVkPipelineLayout(), constant.ShaderStages, constant.Offset, value);
   }


   void VulkanComputeContext::PushConstant(const entt::id_type id, uint32_t value) {
      PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
      const VulkanPushConstant& constant = m_Pipeline->GetPushConstant(id);
      PKZL_CORE_ASSERT(constant.Type == DataType::UInt, "Push constant '{0}' type mismatch.  UInt given, expected {1}!", constant.Name, DataTypeToString(constant.Type));
      GetVkCommandBuffer().pushConstants<uint32_t>(m_Pipeline->GetVkPipelineLayout(), constant.ShaderStages, constant.Offset, value);
   }


   void VulkanComputeContext::PushConstant(const entt::id_type id, float value) {
      PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
      const VulkanPushConstant& constant = m_Pipeline->GetPushConstant(id);
      PKZL_CORE_ASSERT(constant.Type == DataType::Float, "Push constant '{0}' type mismatch.  Float given, expected {1}!", constant.Name, DataTypeToString(constant.Type));
      GetVkCommandBuffer().pushConstants<float>(m_Pipeline->GetVkPipelineLayout(), constant.ShaderStages, constant.Offset, value);
   }


   void VulkanComputeContext::PushConstant(const entt::id_type id, double value) {
      PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
      const VulkanPushConstant& constant = m_Pipeline->GetPushConstant(id);
      PKZL_CORE_ASSERT(constant.Type == DataType::Double, "Push constant '{0}' type mismatch.  Double given, expected {1}!", constant.Name, DataTypeToString(constant.Type));
      GetVkCommandBuffer().pushConstants<double>(m_Pipeline->GetVkPipelineLayout(), constant.ShaderStages, constant.Offset, value);
   }


   void VulkanComputeContext::PushConstant(const entt::id_type id, const glm::bvec2& value) {
      PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
      const VulkanPushConstant& constant = m_Pipeline->GetPushConstant(id);
      PKZL_CORE_ASSERT(constant.Type == DataType::BVec2, "Push constant '{0}' type mismatch.  BVec2 given, expected {1}!", constant.Name, DataTypeToString(constant.Type));
      GetVkCommandBuffer().pushConstants<glm::bvec2>(m_Pipeline->GetVkPipelineLayout(), constant.ShaderStages, constant.Offset, value);
   }


   void VulkanComputeContext::PushConstant(const entt::id_type id, const glm::bvec3& value) {
      PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
      const VulkanPushConstant& constant = m_Pipeline->GetPushConstant(id);
      PKZL_CORE_ASSERT(constant.Type == DataType::BVec3, "Push constant '{0}' type mismatch.  BVec3 given, expected {1}!", constant.Name, DataTypeToString(constant.Type));
      GetVkCommandBuffer().pushConstants<glm::bvec3>(m_Pipeline->GetVkPipelineLayout(), constant.ShaderStages, constant.Offset, value);
   }


   void VulkanComputeContext::PushConstant(const entt::id_type id, const glm::bvec4& value) {
      PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
      const VulkanPushConstant& constant = m_Pipeline->GetPushConstant(id);
      PKZL_CORE_ASSERT(constant.Type == DataType::BVec4, "Push constant '{0}' type mismatch.  BVec4 given, expected {1}!", constant.Name, DataTypeToString(constant.Type));
      GetVkCommandBuffer().pushConstants<glm::bvec4>(m_Pipeline->GetVkPipelineLayout(), constant.ShaderStages, constant.Offset, value);
   }


   void VulkanComputeContext::PushConstant(const entt::id_type id, const glm::ivec2& value) {
      PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
      const VulkanPushConstant& constant = m_Pipeline->GetPushConstant(id);
      PKZL_CORE_ASSERT(constant.Type == DataType::IVec2, "Push constant '{0}' type mismatch.  IVec2 given, expected {1}!", constant.Name, DataTypeToString(constant.Type));
      GetVkCommandBuffer().pushConstants<glm::ivec2>(m_Pipeline->GetVkPipelineLayout(), constant.ShaderStages, constant.Offset, value);
   }


   void VulkanComputeContext::PushConstant(const entt::id_type id, const glm::ivec3& value) {
      PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
      const VulkanPushConstant& constant = m_Pipeline->GetPushConstant(id);
      PKZL_CORE_ASSERT(constant.Type == DataType::IVec3, "Push constant '{0}' type mismatch.  IVec3 given, expected {1}!", constant.Name, DataTypeToString(constant.Type));
      GetVkCommandBuffer().pushConstants<glm::ivec3>(m_Pipeline->GetVkPipelineLayout(), constant.ShaderStages, constant.Offset, value);
   }


   void VulkanComputeContext::PushConstant(const entt::id_type id, const glm::ivec4& value) {
      PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
      const VulkanPushConstant& constant = m_Pipeline->GetPushConstant(id);
      PKZL_CORE_ASSERT(constant.Type == DataType::IVec4, "Push constant '{0}' type mismatch.  IVec4 given, expected {1}!", constant.Name, DataTypeToString(constant.Type));
      GetVkCommandBuffer().pushConstants<glm::ivec4>(m_Pipeline->GetVkPipelineLayout(), constant.ShaderStages, constant.Offset, value);
   }


   void VulkanComputeContext::PushConstant(const entt::id_type id, const glm::uvec2& value) {
      PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
      const VulkanPushConstant& constant = m_Pipeline->GetPushConstant(id);
      PKZL_CORE_ASSERT(constant.Type == DataType::UVec2, "Push constant '{0}' type mismatch.  UVec2 given, expected {1}!", constant.Name, DataTypeToString(constant.Type));
      GetVkCommandBuffer().pushConstants<glm::uvec2>(m_Pipeline->GetVkPipelineLayout(), constant.ShaderStages, constant.Offset, value);
   }


   void VulkanComputeContext::PushConstant(const entt::id_type id, const glm::uvec3& value) {
      PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
      const VulkanPushConstant& constant = m_Pipeline->GetPushConstant(id);
      PKZL_CORE_ASSERT(constant.Type == DataType::UVec3, "Push constant '{0}' type mismatch.  UVec3 given, expected {1}!", constant.Name, DataTypeToString(constant.Type));
      GetVkCommandBuffer().pushConstants<glm::uvec3>(m_Pipeline->GetVkPipelineLayout(), constant.ShaderStages, constant.Offset, value);
   }


   void VulkanComputeContext::PushConstant(const entt::id_type id, const glm::uvec4& value) {
      PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
      const VulkanPushConstant& constant = m_Pipeline->GetPushConstant(id);
      PKZL_CORE_ASSERT(constant.Type == DataType::UVec4, "Push constant '{0}' type mismatch.  UVec4 given, expected {1}!", constant.Name, DataTypeToString(constant.Type));
      GetVkCommandBuffer().pushConstants<glm::uvec4>(m_Pipeline->GetVkPipelineLayout(), constant.ShaderStages, constant.Offset, value);
   }


   void VulkanComputeContext::PushConstant(const entt::id_type id, const glm::vec2& value) {
      PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
      const VulkanPushConstant& constant = m_Pipeline->GetPushConstant(id);
      PKZL_CORE_ASSERT(constant.Type == DataType::Vec2, "Push constant '{0}' type mismatch.  Vec2 given, expected {1}!", constant.Name, DataTypeToString(constant.Type));
      GetVkCommandBuffer().pushConstants<glm::vec2>(m_Pipeline->GetVkPipelineLayout(), constant.ShaderStages, constant.Offset, value);
   }


   void VulkanComputeContext::PushConstant(const entt::id_type id, const glm::vec3& value) {
      PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
      const VulkanPushConstant& constant = m_Pipeline->GetPushConstant(id);
      PKZL_CORE_ASSERT(constant.Type == DataType::Vec3, "Push constant '{0}' type mismatch.  Vec3 given, expected {1}!", constant.Name, DataTypeToString(constant.Type));
      GetVkCommandBuffer().pushConstants<glm::vec3>(m_Pipeline->GetVkPipelineLayout(), constant.ShaderStages, constant.Offset, value);
   }


   void VulkanComputeContext::PushConstant(const entt::id_type id, const glm::vec4& value) {
      PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
      const VulkanPushConstant& constant = m_Pipeline->GetPushConstant(id); // name will be like constants.mvp,  or anotherConstants.something4
      PKZL_CORE_ASSERT(constant.Type == DataType::Vec4, "Push constant '{0}' type mismatch.  Vec4 given, expected {1}!", constant.Name, DataTypeToString(constant.Type));
      GetVkCommandBuffer().pushConstants<glm::vec4>(m_Pipeline->GetVkPipelineLayout(), constant.ShaderStages, constant.Offset, value);
   }


   void VulkanComputeContext::PushConstant(const entt::id_type id, const glm::dvec2& value) {
      PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
      const VulkanPushConstant& constant = m_Pipeline->GetPushConstant(id);
      PKZL_CORE_ASSERT(constant.Type == DataType::DVec2, "Push constant '{0}' type mismatch.  DVec2 given, expected {1}!", constant.Name, DataTypeToString(constant.Type));
      GetVkCommandBuffer().pushConstants<glm::dvec2>(m_Pipeline->GetVkPipelineLayout(), constant.ShaderStages, constant.Offset, value);
   }


   void VulkanComputeContext::PushConstant(const entt::id_type id, const glm::dvec3& value) {
      PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
      const VulkanPushConstant& constant = m_Pipeline->GetPushConstant(id);
      PKZL_CORE_ASSERT(constant.Type == DataType::DVec3, "Push constant '{0}' type mismatch.  DVec3 given, expected {1}!", constant.Name, DataTypeToString(constant.Type));
      GetVkCommandBuffer().pushConstants<glm::dvec3>(m_Pipeline->GetVkPipelineLayout(), constant.ShaderStages, constant.Offset, value);
   }


   void VulkanComputeContext::PushConstant(const entt::id_type id, const glm::dvec4& value) {
      PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
      const VulkanPushConstant& constant = m_Pipeline->GetPushConstant(id);
      PKZL_CORE_ASSERT(constant.Type == DataType::DVec4, "Push constant '{0}' type mismatch.  DVec4 given, expected {1}!", constant.Name, DataTypeToString(constant.Type));
      GetVkCommandBuffer().pushConstants<glm::dvec4>(m_Pipeline->GetVkPipelineLayout(), constant.ShaderStages, constant.Offset, value);
   }


   void VulkanComputeContext::PushConstant(const entt::id_type id, const glm::mat2& value) {
      PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
      const VulkanPushConstant& constant = m_Pipeline->GetPushConstant(id);
      PKZL_CORE_ASSERT(constant.Type == DataType::Mat2, "Push constant '{0}' type mismatch.  Mat2 given, expected {1}!", constant.Name, DataTypeToString(constant.Type));
      GetVkCommandBuffer().pushConstants<glm::mat2>(m_Pipeline->GetVkPipelineLayout(), constant.ShaderStages, constant.Offset, value);
   }


   //void VulkanComputeContext::PushConstant(const entt::id_type id, const glm::mat2x3& value) {
   //   PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
   //   const VulkanPushConstant& constant = m_Pipeline->GetPushConstant(id);
   //   PKZL_CORE_ASSERT(constant.Type == DataType::Mat2x3, "Push constant '{0}' type mismatch.  Mat2x3 given, expected {1}!", constant.Name, DataTypeToString(constant.Type));
   //   GetVkCommandBuffer().pushConstants<glm::mat2x3>(m_Pipeline->GetVkPipelineLayout(), constant.ShaderStages, constant.Offset, value);
   //}


   void VulkanComputeContext::PushConstant(const entt::id_type id, const glm::mat2x4& value) {
      PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
      const VulkanPushConstant& constant = m_Pipeline->GetPushConstant(id);
      PKZL_CORE_ASSERT(constant.Type == DataType::Mat2x4, "Push constant '{0}' type mismatch.  Mat2x4 given, expected {1}!", constant.Name, DataTypeToString(constant.Type));
      GetVkCommandBuffer().pushConstants<glm::mat2x4>(m_Pipeline->GetVkPipelineLayout(), constant.ShaderStages, constant.Offset, value);
   }


   void VulkanComputeContext::PushConstant(const entt::id_type id, const glm::mat3x2& value) {
      PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
      const VulkanPushConstant& constant = m_Pipeline->GetPushConstant(id);
      PKZL_CORE_ASSERT(constant.Type == DataType::Mat3x2, "Push constant '{0}' type mismatch.  Mat3x2 given, expected {1}!", constant.Name, DataTypeToString(constant.Type));
      GetVkCommandBuffer().pushConstants<glm::mat3x2>(m_Pipeline->GetVkPipelineLayout(), constant.ShaderStages, constant.Offset, value);
   }


   //void VulkanComputeContext::PushConstant(const entt::id_type id, const glm::mat3& value) {
   //   PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
   //   const VulkanPushConstant& constant = m_Pipeline->GetPushConstant(id);
   //   PKZL_CORE_ASSERT(constant.Type == DataType::Mat3, "Push constant '{0}' type mismatch.  Mat3 given, expected {1}!", constant.Name, DataTypeToString(constant.Type));
   //   GetVkCommandBuffer().pushConstants<glm::mat3>(m_Pipeline->GetVkPipelineLayout(), constant.ShaderStages, constant.Offset, value);
   //}


   void VulkanComputeContext::PushConstant(const entt::id_type id, const glm::mat3x4& value) {
      PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
      const VulkanPushConstant& constant = m_Pipeline->GetPushConstant(id);
      PKZL_CORE_ASSERT(constant.Type == DataType::Mat3x4, "Push constant '{0}' type mismatch.  Mat3x4 given, expected {1}!", constant.Name, DataTypeToString(constant.Type));
      GetVkCommandBuffer().pushConstants<glm::mat3x4>(m_Pipeline->GetVkPipelineLayout(), constant.ShaderStages, constant.Offset, value);
   }


   void VulkanComputeContext::PushConstant(const entt::id_type id, const glm::mat4x2& value) {
      PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
      const VulkanPushConstant& constant = m_Pipeline->GetPushConstant(id);
      PKZL_CORE_ASSERT(constant.Type == DataType::Mat4x2, "Push constant '{0}' type mismatch.  Mat4x2 given, expected {1}!", constant.Name, DataTypeToString(constant.Type));
      GetVkCommandBuffer().pushConstants<glm::mat4x2>(m_Pipeline->GetVkPipelineLayout(), constant.ShaderStages, constant.Offset, value);
   }


   //void VulkanComputeContext::PushConstant(const entt::id_type id, const glm::mat4x3& value) {
   //   PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
   //   const VulkanPushConstant& constant = m_Pipeline->GetPushConstant(id);
   //   PKZL_CORE_ASSERT(constant.Type == DataType::Mat4x3, "Push constant '{0}' type mismatch.  Mat4x3 given, expected {1}!", constant.Name, DataTypeToString(constant.Type));
   //   m_CommandBuffers[m_CurrentImage].pushConstants<glm::mat4x3>(m_Pipeline->GetVkPipelineLayout(), constant.ShaderStages, constant.Offset, value);
   //}


   void VulkanComputeContext::PushConstant(const entt::id_type id, const glm::mat4& value) {
      PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
      const VulkanPushConstant& constant = m_Pipeline->GetPushConstant(id); // name will be like constants.mvp,  or anotherConstants.something4
      PKZL_CORE_ASSERT(constant.Type == DataType::Mat4, "Push constant '{0}' type mismatch.  Mat4 given, expected {1}!", constant.Name, DataTypeToString(constant.Type));
      GetVkCommandBuffer().pushConstants<glm::mat4>(m_Pipeline->GetVkPipelineLayout(), constant.ShaderStages, constant.Offset, value);
   }


   void VulkanComputeContext::PushConstant(const entt::id_type id, const glm::dmat2& value) {
      PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
      const VulkanPushConstant& constant = m_Pipeline->GetPushConstant(id);
      PKZL_CORE_ASSERT(constant.Type == DataType::DMat2, "Push constant '{0}' type mismatch.  DMat2 given, expected {1}!", constant.Name, DataTypeToString(constant.Type));
      GetVkCommandBuffer().pushConstants<glm::dmat2>(m_Pipeline->GetVkPipelineLayout(), constant.ShaderStages, constant.Offset, value);
   }


   //void VulkanComputeContext::PushConstant(const entt::id_type id, const glm::dmat2x3& value) {
   //   PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
   //   const VulkanPushConstant& constant = m_Pipeline->GetPushConstant(id);
   //   PKZL_CORE_ASSERT(constant.Type == DataType::DMat2x3, "Push constant '{0}' type mismatch.  DMat2x3 given, expected {1}!", constant.Name, DataTypeToString(constant.Type));
   //   m_CommandBuffers[m_CurrentImage].pushConstants<glm::dmat2x3>(m_Pipeline->GetVkPipelineLayout(), constant.ShaderStages, constant.Offset, value);
   //}


   void VulkanComputeContext::PushConstant(const entt::id_type id, const glm::dmat2x4& value) {
      PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
      const VulkanPushConstant& constant = m_Pipeline->GetPushConstant(id);
      PKZL_CORE_ASSERT(constant.Type == DataType::DMat2x4, "Push constant '{0}' type mismatch.  DMat2x4 given, expected {1}!", constant.Name, DataTypeToString(constant.Type));
      GetVkCommandBuffer().pushConstants<glm::dmat2x4>(m_Pipeline->GetVkPipelineLayout(), constant.ShaderStages, constant.Offset, value);
   }


   void VulkanComputeContext::PushConstant(const entt::id_type id, const glm::dmat3x2& value) {
      PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
      const VulkanPushConstant& constant = m_Pipeline->GetPushConstant(id);
      PKZL_CORE_ASSERT(constant.Type == DataType::DMat3x2, "Push constant '{0}' type mismatch.  DMat3x2 given, expected {1}!", constant.Name, DataTypeToString(constant.Type));
      GetVkCommandBuffer().pushConstants<glm::dmat3x2>(m_Pipeline->GetVkPipelineLayout(), constant.ShaderStages, constant.Offset, value);
   }


   //void VulkanComputeContext::PushConstant(const entt::id_type id, const glm::dmat3& value) {
   //   PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
   //   const VulkanPushConstant& constant = m_Pipeline->GetPushConstant(id);
   //   PKZL_CORE_ASSERT(constant.Type == DataType::DMat3, "Push constant '{0}' type mismatch.  DMat3 given, expected {1}!", constant.Name, DataTypeToString(constant.Type));
   //   GetVkCommandBuffer().pushConstants<glm::dmat3>(m_Pipeline->GetVkPipelineLayout(), constant.ShaderStages, constant.Offset, value);
   //}


   void VulkanComputeContext::PushConstant(const entt::id_type id, const glm::dmat3x4& value) {
      PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
      const VulkanPushConstant& constant = m_Pipeline->GetPushConstant(id);
      PKZL_CORE_ASSERT(constant.Type == DataType::DMat3x4, "Push constant '{0}' type mismatch.  DMat3x4 given, expected {1}!", constant.Name, DataTypeToString(constant.Type));
      GetVkCommandBuffer().pushConstants<glm::dmat3x4>(m_Pipeline->GetVkPipelineLayout(), constant.ShaderStages, constant.Offset, value);
   }


   void VulkanComputeContext::PushConstant(const entt::id_type id, const glm::dmat4x2& value) {
      PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
      const VulkanPushConstant& constant = m_Pipeline->GetPushConstant(id);
      PKZL_CORE_ASSERT(constant.Type == DataType::DMat4x2, "Push constant '{0}' type mismatch.  DMat4x2 given, expected {1}!", constant.Name, DataTypeToString(constant.Type));
      GetVkCommandBuffer().pushConstants<glm::dmat4x2>(m_Pipeline->GetVkPipelineLayout(), constant.ShaderStages, constant.Offset, value);
   }


   //void VulkanComputeContext::PushConstant(const entt::id_type id, const glm::dmat4x3& value) {
   //   PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
   //   const VulkanPushConstant& constant = m_Pipeline->GetPushConstant(id);
   //   PKZL_CORE_ASSERT(constant.Type == DataType::DMat4x3, "Push constant '{0}' type mismatch.  DMat4x3 given, expected {1}!", constant.Name, DataTypeToString(constant.Type));
   //   GetVkCommandBuffer().pushConstants<glm::dmat4x3>(m_Pipeline->GetVkPipelineLayout(), constant.ShaderStages, constant.Offset, value);
   //}


   void VulkanComputeContext::PushConstant(const entt::id_type id, const glm::dmat4& value) {
      PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
      const VulkanPushConstant& constant = m_Pipeline->GetPushConstant(id);
      PKZL_CORE_ASSERT(constant.Type == DataType::DMat4, "Push constant '{0}' type mismatch.  DMat4 given, expected {1}!", constant.Name, DataTypeToString(constant.Type));
      GetVkCommandBuffer().pushConstants<glm::dmat4>(m_Pipeline->GetVkPipelineLayout(), constant.ShaderStages, constant.Offset, value);
   }


   void VulkanComputeContext::Dispatch(const uint32_t x, const uint32_t y, const uint32_t z) {
      BindDescriptorSets();
      GetVkCommandBuffer().dispatch(x, y, z);
   }


   vk::PipelineCache VulkanComputeContext::GetVkPipelineCache() const {
      return m_PipelineCache;
   }


   vk::CommandBuffer VulkanComputeContext::GetVkCommandBuffer() {
      return m_CommandBuffers.front();
   }


   std::shared_ptr<VulkanFence> VulkanComputeContext::GetFence() {
      return m_InFlightFence;
   }


   vk::DescriptorPool VulkanComputeContext::CreateDescriptorPool(const vk::ArrayProxy<const DescriptorBinding>& descriptorBindings, size_t maxSets) {
      std::vector<vk::DescriptorPoolSize> poolSizes;
      poolSizes.reserve(descriptorBindings.size());

      for (const auto& binding : descriptorBindings) {
         poolSizes.emplace_back(binding.Type, static_cast<uint32_t>(binding.DescriptorCount * maxSets));
      }

      vk::DescriptorPoolCreateInfo descriptorPoolCI = {
         vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet   /*flags*/,
         static_cast<uint32_t>(maxSets)                         /*maxSets*/,
         static_cast<uint32_t>(poolSizes.size())                /*poolSizeCount*/,
         poolSizes.data()                                       /*pPoolSizes*/
      };
      return m_Device->GetVkDevice().createDescriptorPool(descriptorPoolCI);
   }


   void VulkanComputeContext::DestroyDescriptorPool(vk::DescriptorPool descriptorPool) {
      if (m_Device) {
         if (descriptorPool) {
            m_Device->GetVkDevice().destroy(descriptorPool);
            descriptorPool = nullptr;
         }
      }
   }


   void VulkanComputeContext::CreateCommandPool() {
      m_CommandPool = m_Device->GetVkDevice().createCommandPool({
         vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
         m_Device->GetGraphicsQueueFamilyIndex()
      });
   }


   void VulkanComputeContext::DestroyCommandPool() {
      if (m_Device && m_CommandPool) {
         m_Device->GetVkDevice().destroy(m_CommandPool);
         m_CommandPool = nullptr;
      }
   }


   void VulkanComputeContext::CreateCommandBuffers(const uint32_t commandBufferCount) {
      m_CommandBuffers = m_Device->GetVkDevice().allocateCommandBuffers({
         m_CommandPool                      /*commandPool*/,
         vk::CommandBufferLevel::ePrimary   /*level*/,
         commandBufferCount                 /*commandBufferCount*/
         });
   }


   void VulkanComputeContext::DestroyCommandBuffers() {
      if (m_Device && m_CommandPool) {
         m_Device->GetVkDevice().freeCommandBuffers(m_CommandPool, m_CommandBuffers);
         m_CommandBuffers.clear();
      }
   }


   void VulkanComputeContext::CreatePipelineCache() {
      m_PipelineCache = m_Device->GetVkDevice().createPipelineCache({});
   }


   void VulkanComputeContext::DestroyPipelineCache() {
      if (m_Device && m_PipelineCache) {
         m_Device->GetVkDevice().destroy(m_PipelineCache);
      }
   }


   void VulkanComputeContext::CreateSyncObjects() {
      m_InFlightFence = std::make_shared<VulkanFence>(m_Device->GetVkDevice());
   }


   void VulkanComputeContext::DestroySyncObjects() {
      if (m_Device && m_InFlightFence) {
         m_InFlightFence = nullptr;
      }
   }


   void VulkanComputeContext::BindDescriptorSets() {
      m_Pipeline->BindDescriptorSets(GetVkCommandBuffer(), GetFence());
   }


   void VulkanComputeContext::UnbindDescriptorSets() {
      m_Pipeline->UnbindDescriptorSets();
   }

}
