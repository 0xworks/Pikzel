#include "VulkanGraphicsContext.h"

#include "SwapChainSupportDetails.h"
#include "VulkanBuffer.h"
#include "VulkanPipeline.h"
#include "VulkanTexture.h"
#include "VulkanUtility.h"

#include "Pikzel/Events/EventDispatcher.h"

#include <imgui.h>
#include <imgui_internal.h>
#include <backends/imgui_impl_glfw.h>
#include "imgui_impl_vulkan.h"

namespace Pikzel {

   VulkanGraphicsContext::VulkanGraphicsContext(std::shared_ptr<VulkanDevice> device) : m_Device {device} {}


   VulkanGraphicsContext::~VulkanGraphicsContext() {}


   void VulkanGraphicsContext::InitializeImGui() {
       __super::InitializeImGui();
   }


   ImGuiContext* VulkanGraphicsContext::GetImGuiContext() {
      return ImGui::GetCurrentContext();
   }


   void VulkanGraphicsContext::BeginImGuiFrame() {}
   void VulkanGraphicsContext::EndImGuiFrame() {}


   void VulkanGraphicsContext::Bind(const VertexBuffer& buffer) {
      const VulkanVertexBuffer& vulkanVertexBuffer = static_cast<const VulkanVertexBuffer&>(buffer);
      GetVkCommandBuffer().bindVertexBuffers(0, vulkanVertexBuffer.GetVkBuffer(), {0});
   }


   void VulkanGraphicsContext::Unbind(const VertexBuffer& buffer) {}


   void VulkanGraphicsContext::Bind(const IndexBuffer& buffer) {
      const VulkanIndexBuffer& vulkanIndexBuffer = static_cast<const VulkanIndexBuffer&>(buffer);
      GetVkCommandBuffer().bindIndexBuffer(vulkanIndexBuffer.GetVkBuffer(), 0, vk::IndexType::eUint32);
   }


   void VulkanGraphicsContext::Unbind(const IndexBuffer& buffer) {}


   void VulkanGraphicsContext::Bind(const UniformBuffer& buffer, const entt::id_type resourceId) {
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
         resource.Count                                          /*descriptorCount*/,
         resource.Type                                           /*descriptorType*/,
         nullptr                                                 /*pImageInfo*/,
         &uniformBufferDescriptor                                /*pBufferInfo*/,
         nullptr                                                 /*pTexelBufferView*/
      };

      m_Device->GetVkDevice().updateDescriptorSets(uniformBufferWrite, nullptr);
   }


   void VulkanGraphicsContext::Unbind(const UniformBuffer&) {}


   void VulkanGraphicsContext::Bind(const Texture2D& texture, const entt::id_type resourceId) {
      const VulkanResource& resource = m_Pipeline->GetResource(resourceId);

      vk::DescriptorImageInfo textureImageDescriptor = {
         static_cast<const VulkanTexture2D&>(texture).GetVkSampler()   /*sampler*/,
         static_cast<const VulkanTexture2D&>(texture).GetVkImageView() /*imageView*/,
         vk::ImageLayout::eShaderReadOnlyOptimal                       /*imageLayout*/
      };

      vk::WriteDescriptorSet textureSamplersWrite = {
         m_Pipeline->GetVkDescriptorSet(resource.DescriptorSet)  /*dstSet*/,
         resource.Binding                                        /*dstBinding*/,
         0                                                       /*dstArrayElement*/,
         resource.Count                                          /*descriptorCount*/,
         resource.Type                                           /*descriptorType*/,
         &textureImageDescriptor                                 /*pImageInfo*/,
         nullptr                                                 /*pBufferInfo*/,
         nullptr                                                 /*pTexelBufferView*/
      };

      m_Device->GetVkDevice().updateDescriptorSets(textureSamplersWrite, nullptr);
   }


   void VulkanGraphicsContext::Unbind(const Texture2D&) {}


   void VulkanGraphicsContext::Bind(const TextureCube& texture, const entt::id_type resourceId) {
      const VulkanResource& resource = m_Pipeline->GetResource(resourceId);

      vk::DescriptorImageInfo textureImageDescriptor = {
         static_cast<const VulkanTextureCube&>(texture).GetVkSampler()   /*sampler*/,
         static_cast<const VulkanTextureCube&>(texture).GetVkImageView() /*imageView*/,
         vk::ImageLayout::eShaderReadOnlyOptimal                         /*imageLayout*/
      };

      vk::WriteDescriptorSet textureSamplersWrite = {
         m_Pipeline->GetVkDescriptorSet(resource.DescriptorSet)  /*dstSet*/,
         resource.Binding                                        /*dstBinding*/,
         0                                                       /*dstArrayElement*/,
         resource.Count                                          /*descriptorCount*/,
         resource.Type                                           /*descriptorType*/,
         &textureImageDescriptor                                 /*pImageInfo*/,
         nullptr                                                 /*pBufferInfo*/,
         nullptr                                                 /*pTexelBufferView*/
      };

      m_Device->GetVkDevice().updateDescriptorSets(textureSamplersWrite, nullptr);
   }


   void VulkanGraphicsContext::Unbind(const TextureCube&) {}


   std::unique_ptr<Framebuffer> VulkanGraphicsContext::CreateFramebuffer(const FramebufferSettings& settings) {
      return std::make_unique<VulkanFramebuffer>(m_Device, m_RenderPass, settings);
   }


   std::unique_ptr<Pikzel::Pipeline> VulkanGraphicsContext::CreatePipeline(const PipelineSettings& settings) {
      return std::make_unique<VulkanPipeline>(m_Device, *this, settings);
   }


   void VulkanGraphicsContext::PushConstant(const entt::id_type id, bool value) {
      PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
      const VulkanPushConstant& constant = m_Pipeline->GetPushConstant(id);
      PKZL_CORE_ASSERT(constant.Type == DataType::Bool, "Push constant '{0}' type mismatch.  Bool given, expected {1}!", constant.Name, DataTypeToString(constant.Type));
      GetVkCommandBuffer().pushConstants<bool>(m_Pipeline->GetVkPipelineLayout(), constant.ShaderStages, constant.Offset, value);

   }


   void VulkanGraphicsContext::PushConstant(const entt::id_type id, int value) {
      PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
      const VulkanPushConstant& constant = m_Pipeline->GetPushConstant(id);
      PKZL_CORE_ASSERT(constant.Type == DataType::Int, "Push constant '{0}' type mismatch.  Int given, expected {1}!", constant.Name, DataTypeToString(constant.Type));
      GetVkCommandBuffer().pushConstants<int>(m_Pipeline->GetVkPipelineLayout(), constant.ShaderStages, constant.Offset, value);
   }


   void VulkanGraphicsContext::PushConstant(const entt::id_type id, uint32_t value) {
      PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
      const VulkanPushConstant& constant = m_Pipeline->GetPushConstant(id);
      PKZL_CORE_ASSERT(constant.Type == DataType::UInt, "Push constant '{0}' type mismatch.  UInt given, expected {1}!", constant.Name, DataTypeToString(constant.Type));
      GetVkCommandBuffer().pushConstants<uint32_t>(m_Pipeline->GetVkPipelineLayout(), constant.ShaderStages, constant.Offset, value);
   }


   void VulkanGraphicsContext::PushConstant(const entt::id_type id, float value) {
      PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
      const VulkanPushConstant& constant = m_Pipeline->GetPushConstant(id);
      PKZL_CORE_ASSERT(constant.Type == DataType::Float, "Push constant '{0}' type mismatch.  Float given, expected {1}!", constant.Name, DataTypeToString(constant.Type));
      GetVkCommandBuffer().pushConstants<float>(m_Pipeline->GetVkPipelineLayout(), constant.ShaderStages, constant.Offset, value);
   }


   void VulkanGraphicsContext::PushConstant(const entt::id_type id, double value) {
      PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
      const VulkanPushConstant& constant = m_Pipeline->GetPushConstant(id);
      PKZL_CORE_ASSERT(constant.Type == DataType::Double, "Push constant '{0}' type mismatch.  Double given, expected {1}!", constant.Name, DataTypeToString(constant.Type));
      GetVkCommandBuffer().pushConstants<double>(m_Pipeline->GetVkPipelineLayout(), constant.ShaderStages, constant.Offset, value);
   }


   void VulkanGraphicsContext::PushConstant(const entt::id_type id, const glm::bvec2& value) {
      PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
      const VulkanPushConstant& constant = m_Pipeline->GetPushConstant(id);
      PKZL_CORE_ASSERT(constant.Type == DataType::BVec2, "Push constant '{0}' type mismatch.  BVec2 given, expected {1}!", constant.Name, DataTypeToString(constant.Type));
      GetVkCommandBuffer().pushConstants<glm::bvec2>(m_Pipeline->GetVkPipelineLayout(), constant.ShaderStages, constant.Offset, value);
   }


   void VulkanGraphicsContext::PushConstant(const entt::id_type id, const glm::bvec3& value) {
      PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
      const VulkanPushConstant& constant = m_Pipeline->GetPushConstant(id);
      PKZL_CORE_ASSERT(constant.Type == DataType::BVec3, "Push constant '{0}' type mismatch.  BVec3 given, expected {1}!", constant.Name, DataTypeToString(constant.Type));
      GetVkCommandBuffer().pushConstants<glm::bvec3>(m_Pipeline->GetVkPipelineLayout(), constant.ShaderStages, constant.Offset, value);
   }


   void VulkanGraphicsContext::PushConstant(const entt::id_type id, const glm::bvec4& value) {
      PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
      const VulkanPushConstant& constant = m_Pipeline->GetPushConstant(id);
      PKZL_CORE_ASSERT(constant.Type == DataType::BVec4, "Push constant '{0}' type mismatch.  BVec4 given, expected {1}!", constant.Name, DataTypeToString(constant.Type));
      GetVkCommandBuffer().pushConstants<glm::bvec4>(m_Pipeline->GetVkPipelineLayout(), constant.ShaderStages, constant.Offset, value);
   }


   void VulkanGraphicsContext::PushConstant(const entt::id_type id, const glm::ivec2& value) {
      PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
      const VulkanPushConstant& constant = m_Pipeline->GetPushConstant(id);
      PKZL_CORE_ASSERT(constant.Type == DataType::IVec2, "Push constant '{0}' type mismatch.  IVec2 given, expected {1}!", constant.Name, DataTypeToString(constant.Type));
      GetVkCommandBuffer().pushConstants<glm::ivec2>(m_Pipeline->GetVkPipelineLayout(), constant.ShaderStages, constant.Offset, value);
   }


   void VulkanGraphicsContext::PushConstant(const entt::id_type id, const glm::ivec3& value) {
      PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
      const VulkanPushConstant& constant = m_Pipeline->GetPushConstant(id);
      PKZL_CORE_ASSERT(constant.Type == DataType::IVec3, "Push constant '{0}' type mismatch.  IVec3 given, expected {1}!", constant.Name, DataTypeToString(constant.Type));
      GetVkCommandBuffer().pushConstants<glm::ivec3>(m_Pipeline->GetVkPipelineLayout(), constant.ShaderStages, constant.Offset, value);
   }


   void VulkanGraphicsContext::PushConstant(const entt::id_type id, const glm::ivec4& value) {
      PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
      const VulkanPushConstant& constant = m_Pipeline->GetPushConstant(id);
      PKZL_CORE_ASSERT(constant.Type == DataType::IVec4, "Push constant '{0}' type mismatch.  IVec4 given, expected {1}!", constant.Name, DataTypeToString(constant.Type));
      GetVkCommandBuffer().pushConstants<glm::ivec4>(m_Pipeline->GetVkPipelineLayout(), constant.ShaderStages, constant.Offset, value);
   }


   void VulkanGraphicsContext::PushConstant(const entt::id_type id, const glm::uvec2& value) {
      PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
      const VulkanPushConstant& constant = m_Pipeline->GetPushConstant(id);
      PKZL_CORE_ASSERT(constant.Type == DataType::UVec2, "Push constant '{0}' type mismatch.  UVec2 given, expected {1}!", constant.Name, DataTypeToString(constant.Type));
      GetVkCommandBuffer().pushConstants<glm::uvec2>(m_Pipeline->GetVkPipelineLayout(), constant.ShaderStages, constant.Offset, value);
   }


   void VulkanGraphicsContext::PushConstant(const entt::id_type id, const glm::uvec3& value) {
      PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
      const VulkanPushConstant& constant = m_Pipeline->GetPushConstant(id);
      PKZL_CORE_ASSERT(constant.Type == DataType::UVec3, "Push constant '{0}' type mismatch.  UVec3 given, expected {1}!", constant.Name, DataTypeToString(constant.Type));
      GetVkCommandBuffer().pushConstants<glm::uvec3>(m_Pipeline->GetVkPipelineLayout(), constant.ShaderStages, constant.Offset, value);
   }


   void VulkanGraphicsContext::PushConstant(const entt::id_type id, const glm::uvec4& value) {
      PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
      const VulkanPushConstant& constant = m_Pipeline->GetPushConstant(id);
      PKZL_CORE_ASSERT(constant.Type == DataType::UVec4, "Push constant '{0}' type mismatch.  UVec4 given, expected {1}!", constant.Name, DataTypeToString(constant.Type));
      GetVkCommandBuffer().pushConstants<glm::uvec4>(m_Pipeline->GetVkPipelineLayout(), constant.ShaderStages, constant.Offset, value);
   }


   void VulkanGraphicsContext::PushConstant(const entt::id_type id, const glm::vec2& value) {
      PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
      const VulkanPushConstant& constant = m_Pipeline->GetPushConstant(id);
      PKZL_CORE_ASSERT(constant.Type == DataType::Vec2, "Push constant '{0}' type mismatch.  Vec2 given, expected {1}!", constant.Name, DataTypeToString(constant.Type));
      GetVkCommandBuffer().pushConstants<glm::vec2>(m_Pipeline->GetVkPipelineLayout(), constant.ShaderStages, constant.Offset, value);
   }


   void VulkanGraphicsContext::PushConstant(const entt::id_type id, const glm::vec3& value) {
      PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
      const VulkanPushConstant& constant = m_Pipeline->GetPushConstant(id);
      PKZL_CORE_ASSERT(constant.Type == DataType::Vec3, "Push constant '{0}' type mismatch.  Vec3 given, expected {1}!", constant.Name, DataTypeToString(constant.Type));
      GetVkCommandBuffer().pushConstants<glm::vec3>(m_Pipeline->GetVkPipelineLayout(), constant.ShaderStages, constant.Offset, value);
   }


   void VulkanGraphicsContext::PushConstant(const entt::id_type id, const glm::vec4& value) {
      PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
      const VulkanPushConstant& constant = m_Pipeline->GetPushConstant(id); // name will be like constants.mvp,  or anotherConstants.something4
      PKZL_CORE_ASSERT(constant.Type == DataType::Vec4, "Push constant '{0}' type mismatch.  Vec4 given, expected {1}!", constant.Name, DataTypeToString(constant.Type));
      GetVkCommandBuffer().pushConstants<glm::vec4>(m_Pipeline->GetVkPipelineLayout(), constant.ShaderStages, constant.Offset, value);
   }


   void VulkanGraphicsContext::PushConstant(const entt::id_type id, const glm::dvec2& value) {
      PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
      const VulkanPushConstant& constant = m_Pipeline->GetPushConstant(id);
      PKZL_CORE_ASSERT(constant.Type == DataType::DVec2, "Push constant '{0}' type mismatch.  DVec2 given, expected {1}!", constant.Name, DataTypeToString(constant.Type));
      GetVkCommandBuffer().pushConstants<glm::dvec2>(m_Pipeline->GetVkPipelineLayout(), constant.ShaderStages, constant.Offset, value);
   }


   void VulkanGraphicsContext::PushConstant(const entt::id_type id, const glm::dvec3& value) {
      PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
      const VulkanPushConstant& constant = m_Pipeline->GetPushConstant(id);
      PKZL_CORE_ASSERT(constant.Type == DataType::DVec3, "Push constant '{0}' type mismatch.  DVec3 given, expected {1}!", constant.Name, DataTypeToString(constant.Type));
      GetVkCommandBuffer().pushConstants<glm::dvec3>(m_Pipeline->GetVkPipelineLayout(), constant.ShaderStages, constant.Offset, value);
   }


   void VulkanGraphicsContext::PushConstant(const entt::id_type id, const glm::dvec4& value) {
      PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
      const VulkanPushConstant& constant = m_Pipeline->GetPushConstant(id);
      PKZL_CORE_ASSERT(constant.Type == DataType::DVec4, "Push constant '{0}' type mismatch.  DVec4 given, expected {1}!", constant.Name, DataTypeToString(constant.Type));
      GetVkCommandBuffer().pushConstants<glm::dvec4>(m_Pipeline->GetVkPipelineLayout(), constant.ShaderStages, constant.Offset, value);
   }


   void VulkanGraphicsContext::PushConstant(const entt::id_type id, const glm::mat2& value) {
      PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
      const VulkanPushConstant& constant = m_Pipeline->GetPushConstant(id);
      PKZL_CORE_ASSERT(constant.Type == DataType::Mat2, "Push constant '{0}' type mismatch.  Mat2 given, expected {1}!", constant.Name, DataTypeToString(constant.Type));
      GetVkCommandBuffer().pushConstants<glm::mat2>(m_Pipeline->GetVkPipelineLayout(), constant.ShaderStages, constant.Offset, value);
   }


   //void VulkanGraphicsContext::PushConstant(const entt::id_type id, const glm::mat2x3& value) {
   //   PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
   //   const VulkanPushConstant& constant = m_Pipeline->GetPushConstant(id);
   //   PKZL_CORE_ASSERT(constant.Type == DataType::Mat2x3, "Push constant '{0}' type mismatch.  Mat2x3 given, expected {1}!", constant.Name, DataTypeToString(constant.Type));
   //   GetVkCommandBuffer().pushConstants<glm::mat2x3>(m_Pipeline->GetVkPipelineLayout(), constant.ShaderStages, constant.Offset, value);
   //}


   void VulkanGraphicsContext::PushConstant(const entt::id_type id, const glm::mat2x4& value) {
      PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
      const VulkanPushConstant& constant = m_Pipeline->GetPushConstant(id);
      PKZL_CORE_ASSERT(constant.Type == DataType::Mat2x4, "Push constant '{0}' type mismatch.  Mat2x4 given, expected {1}!", constant.Name, DataTypeToString(constant.Type));
      GetVkCommandBuffer().pushConstants<glm::mat2x4>(m_Pipeline->GetVkPipelineLayout(), constant.ShaderStages, constant.Offset, value);
   }


   void VulkanGraphicsContext::PushConstant(const entt::id_type id, const glm::mat3x2& value) {
      PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
      const VulkanPushConstant& constant = m_Pipeline->GetPushConstant(id);
      PKZL_CORE_ASSERT(constant.Type == DataType::Mat3x2, "Push constant '{0}' type mismatch.  Mat3x2 given, expected {1}!", constant.Name, DataTypeToString(constant.Type));
      GetVkCommandBuffer().pushConstants<glm::mat3x2>(m_Pipeline->GetVkPipelineLayout(), constant.ShaderStages, constant.Offset, value);
   }


   //void VulkanGraphicsContext::PushConstant(const entt::id_type id, const glm::mat3& value) {
   //   PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
   //   const VulkanPushConstant& constant = m_Pipeline->GetPushConstant(id);
   //   PKZL_CORE_ASSERT(constant.Type == DataType::Mat3, "Push constant '{0}' type mismatch.  Mat3 given, expected {1}!", constant.Name, DataTypeToString(constant.Type));
   //   GetVkCommandBuffer().pushConstants<glm::mat3>(m_Pipeline->GetVkPipelineLayout(), constant.ShaderStages, constant.Offset, value);
   //}


   void VulkanGraphicsContext::PushConstant(const entt::id_type id, const glm::mat3x4& value) {
      PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
      const VulkanPushConstant& constant = m_Pipeline->GetPushConstant(id);
      PKZL_CORE_ASSERT(constant.Type == DataType::Mat3x4, "Push constant '{0}' type mismatch.  Mat3x4 given, expected {1}!", constant.Name, DataTypeToString(constant.Type));
      GetVkCommandBuffer().pushConstants<glm::mat3x4>(m_Pipeline->GetVkPipelineLayout(), constant.ShaderStages, constant.Offset, value);
   }


   void VulkanGraphicsContext::PushConstant(const entt::id_type id, const glm::mat4x2& value) {
      PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
      const VulkanPushConstant& constant = m_Pipeline->GetPushConstant(id);
      PKZL_CORE_ASSERT(constant.Type == DataType::Mat4x2, "Push constant '{0}' type mismatch.  Mat4x2 given, expected {1}!", constant.Name, DataTypeToString(constant.Type));
      GetVkCommandBuffer().pushConstants<glm::mat4x2>(m_Pipeline->GetVkPipelineLayout(), constant.ShaderStages, constant.Offset, value);
   }


   //void VulkanGraphicsContext::PushConstant(const entt::id_type id, const glm::mat4x3& value) {
   //   PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
   //   const VulkanPushConstant& constant = m_Pipeline->GetPushConstant(id);
   //   PKZL_CORE_ASSERT(constant.Type == DataType::Mat4x3, "Push constant '{0}' type mismatch.  Mat4x3 given, expected {1}!", constant.Name, DataTypeToString(constant.Type));
   //   m_CommandBuffers[m_CurrentImage].pushConstants<glm::mat4x3>(m_Pipeline->GetVkPipelineLayout(), constant.ShaderStages, constant.Offset, value);
   //}


   void VulkanGraphicsContext::PushConstant(const entt::id_type id, const glm::mat4& value) {
      PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
      const VulkanPushConstant& constant = m_Pipeline->GetPushConstant(id); // name will be like constants.mvp,  or anotherConstants.something4
      PKZL_CORE_ASSERT(constant.Type == DataType::Mat4, "Push constant '{0}' type mismatch.  Mat4 given, expected {1}!", constant.Name, DataTypeToString(constant.Type));
      GetVkCommandBuffer().pushConstants<glm::mat4>(m_Pipeline->GetVkPipelineLayout(), constant.ShaderStages, constant.Offset, value);
   }


   void VulkanGraphicsContext::PushConstant(const entt::id_type id, const glm::dmat2& value) {
      PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
      const VulkanPushConstant& constant = m_Pipeline->GetPushConstant(id);
      PKZL_CORE_ASSERT(constant.Type == DataType::DMat2, "Push constant '{0}' type mismatch.  DMat2 given, expected {1}!", constant.Name, DataTypeToString(constant.Type));
      GetVkCommandBuffer().pushConstants<glm::dmat2>(m_Pipeline->GetVkPipelineLayout(), constant.ShaderStages, constant.Offset, value);
   }


   //void VulkanGraphicsContext::PushConstant(const entt::id_type id, const glm::dmat2x3& value) {
   //   PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
   //   const VulkanPushConstant& constant = m_Pipeline->GetPushConstant(id);
   //   PKZL_CORE_ASSERT(constant.Type == DataType::DMat2x3, "Push constant '{0}' type mismatch.  DMat2x3 given, expected {1}!", constant.Name, DataTypeToString(constant.Type));
   //   m_CommandBuffers[m_CurrentImage].pushConstants<glm::dmat2x3>(m_Pipeline->GetVkPipelineLayout(), constant.ShaderStages, constant.Offset, value);
   //}


   void VulkanGraphicsContext::PushConstant(const entt::id_type id, const glm::dmat2x4& value) {
      PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
      const VulkanPushConstant& constant = m_Pipeline->GetPushConstant(id);
      PKZL_CORE_ASSERT(constant.Type == DataType::DMat2x4, "Push constant '{0}' type mismatch.  DMat2x4 given, expected {1}!", constant.Name, DataTypeToString(constant.Type));
      GetVkCommandBuffer().pushConstants<glm::dmat2x4>(m_Pipeline->GetVkPipelineLayout(), constant.ShaderStages, constant.Offset, value);
   }


   void VulkanGraphicsContext::PushConstant(const entt::id_type id, const glm::dmat3x2& value) {
      PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
      const VulkanPushConstant& constant = m_Pipeline->GetPushConstant(id);
      PKZL_CORE_ASSERT(constant.Type == DataType::DMat3x2, "Push constant '{0}' type mismatch.  DMat3x2 given, expected {1}!", constant.Name, DataTypeToString(constant.Type));
      GetVkCommandBuffer().pushConstants<glm::dmat3x2>(m_Pipeline->GetVkPipelineLayout(), constant.ShaderStages, constant.Offset, value);
   }


   //void VulkanGraphicsContext::PushConstant(const entt::id_type id, const glm::dmat3& value) {
   //   PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
   //   const VulkanPushConstant& constant = m_Pipeline->GetPushConstant(id);
   //   PKZL_CORE_ASSERT(constant.Type == DataType::DMat3, "Push constant '{0}' type mismatch.  DMat3 given, expected {1}!", constant.Name, DataTypeToString(constant.Type));
   //   GetVkCommandBuffer().pushConstants<glm::dmat3>(m_Pipeline->GetVkPipelineLayout(), constant.ShaderStages, constant.Offset, value);
   //}


   void VulkanGraphicsContext::PushConstant(const entt::id_type id, const glm::dmat3x4& value) {
      PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
      const VulkanPushConstant& constant = m_Pipeline->GetPushConstant(id);
      PKZL_CORE_ASSERT(constant.Type == DataType::DMat3x4, "Push constant '{0}' type mismatch.  DMat3x4 given, expected {1}!", constant.Name, DataTypeToString(constant.Type));
      GetVkCommandBuffer().pushConstants<glm::dmat3x4>(m_Pipeline->GetVkPipelineLayout(), constant.ShaderStages, constant.Offset, value);
   }


   void VulkanGraphicsContext::PushConstant(const entt::id_type id, const glm::dmat4x2& value) {
      PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
      const VulkanPushConstant& constant = m_Pipeline->GetPushConstant(id);
      PKZL_CORE_ASSERT(constant.Type == DataType::DMat4x2, "Push constant '{0}' type mismatch.  DMat4x2 given, expected {1}!", constant.Name, DataTypeToString(constant.Type));
      GetVkCommandBuffer().pushConstants<glm::dmat4x2>(m_Pipeline->GetVkPipelineLayout(), constant.ShaderStages, constant.Offset, value);
   }


   //void VulkanGraphicsContext::PushConstant(const entt::id_type id, const glm::dmat4x3& value) {
   //   PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
   //   const VulkanPushConstant& constant = m_Pipeline->GetPushConstant(id);
   //   PKZL_CORE_ASSERT(constant.Type == DataType::DMat4x3, "Push constant '{0}' type mismatch.  DMat4x3 given, expected {1}!", constant.Name, DataTypeToString(constant.Type));
   //   GetVkCommandBuffer().pushConstants<glm::dmat4x3>(m_Pipeline->GetVkPipelineLayout(), constant.ShaderStages, constant.Offset, value);
   //}


   void VulkanGraphicsContext::PushConstant(const entt::id_type id, const glm::dmat4& value) {
      PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
      const VulkanPushConstant& constant = m_Pipeline->GetPushConstant(id);
      PKZL_CORE_ASSERT(constant.Type == DataType::DMat4, "Push constant '{0}' type mismatch.  DMat4 given, expected {1}!", constant.Name, DataTypeToString(constant.Type));
      GetVkCommandBuffer().pushConstants<glm::dmat4>(m_Pipeline->GetVkPipelineLayout(), constant.ShaderStages, constant.Offset, value);
   }


   void VulkanGraphicsContext::DrawTriangles(const VertexBuffer& vertexBuffer, const uint32_t vertexCount, const uint32_t vertexOffset/*= 0*/) {
      m_Pipeline->BindDescriptorSets(GetVkCommandBuffer(), GetVkFence());
      Bind(vertexBuffer);
      GetVkCommandBuffer().draw(vertexCount, 1, vertexOffset, 0);
   }


   void VulkanGraphicsContext::DrawIndexed(const VertexBuffer& vertexBuffer, const IndexBuffer& indexBuffer, const uint32_t indexCount, const uint32_t vertexOffset/*= 0*/) {
      uint32_t count = indexCount ? indexCount : indexBuffer.GetCount();
      m_Pipeline->BindDescriptorSets(GetVkCommandBuffer(), GetVkFence());
      Bind(vertexBuffer);
      Bind(indexBuffer);
      GetVkCommandBuffer().drawIndexed(count, 1, 0, vertexOffset, 0);
   }


   vk::RenderPass VulkanGraphicsContext::GetVkRenderPass() const {
      return m_RenderPass;
   }


   vk::PipelineCache VulkanGraphicsContext::GetVkPipelineCache() const {
      return m_PipelineCache;
   }


   void VulkanGraphicsContext::CreateDepthStencil() {
      // TODO anti-aliasing
      m_DepthFormat = FindSupportedFormat(
         m_Device->GetVkPhysicalDevice(),
         {vk::Format::eD32SfloatS8Uint, vk::Format::eD32Sfloat, vk::Format::eD24UnormS8Uint, vk::Format::eD16UnormS8Uint, vk::Format::eD16Unorm},
         vk::ImageTiling::eOptimal,
         vk::FormatFeatureFlagBits::eDepthStencilAttachment
      );
      m_DepthImage = std::make_unique<VulkanImage>(m_Device, m_Extent.width, m_Extent.height, 1, vk::SampleCountFlagBits::e1, m_DepthFormat, vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eDepthStencilAttachment, vk::MemoryPropertyFlagBits::eDeviceLocal, vk::ImageCreateFlags {});
      m_DepthImage->CreateImageView(m_DepthFormat, vk::ImageAspectFlagBits::eDepth);
   }


   void VulkanGraphicsContext::DestroyDepthStencil() {
      m_DepthImage = nullptr;
   }


   vk::RenderPass VulkanGraphicsContext::CreateRenderPass(const bool clearColorBuffer, const bool clearDepthBuffer, vk::ImageLayout finalLayout) {
      std::vector<vk::AttachmentDescription> attachments = {
         {
            {}                                                                                /*flags*/,
            m_Format                                                                          /*format*/,
            vk::SampleCountFlagBits::e1                                                       /*samples*/,   // TODO: anti-aliasing
            clearColorBuffer? vk::AttachmentLoadOp::eClear : vk::AttachmentLoadOp::eDontCare  /*loadOp*/,
            vk::AttachmentStoreOp::eStore                                                     /*storeOp*/,
            vk::AttachmentLoadOp::eDontCare                                                   /*stencilLoadOp*/,
            vk::AttachmentStoreOp::eDontCare                                                  /*stencilStoreOp*/,
            vk::ImageLayout::eUndefined                                                       /*initialLayout*/,
            finalLayout                                                                       /*finalLayout*/     // anti-aliasing = vk::ImageLayout::eColorAttachmentOptimal here
         },
         {
            {}                                                                                /*flags*/,
            m_DepthFormat                                                                     /*format*/,
            vk::SampleCountFlagBits::e1                                                       /*samples*/,   // TODO: anti-aliasing
            clearDepthBuffer? vk::AttachmentLoadOp::eClear : vk::AttachmentLoadOp::eDontCare  /*loadOp*/,
            vk::AttachmentStoreOp::eDontCare                                                  /*storeOp*/,
            vk::AttachmentLoadOp::eDontCare                                                   /*stencilLoadOp*/,
            vk::AttachmentStoreOp::eDontCare                                                  /*stencilStoreOp*/,
            vk::ImageLayout::eUndefined                                                       /*initialLayout*/,
            vk::ImageLayout::eDepthStencilAttachmentOptimal                                   /*finalLayout*/
         }
         //{
         //    {}                                              /*flags*/,
         //    format                                          /*format*/,
         //    vk::SampleCountFlagBits::e1                     /*samples*/,
         //    vk::AttachmentLoadOp::eDontCare                 /*loadOp*/,
         //    vk::AttachmentStoreOp::eStore                   /*storeOp*/,
         //    vk::AttachmentLoadOp::eDontCare                 /*stencilLoadOp*/,
         //    vk::AttachmentStoreOp::eDontCare                /*stencilStoreOp*/,
         //    vk::ImageLayout::eUndefined                     /*initialLayout*/,
         //    vk::ImageLayout::ePresentSrcKHR                 /*finalLayout*/
         //}
      };

      vk::AttachmentReference colorAttachmentRef = {
         0,
         vk::ImageLayout::eColorAttachmentOptimal
      };

      vk::AttachmentReference depthAttachmentRef = {
         1,
         vk::ImageLayout::eDepthStencilAttachmentOptimal
      };

      //    vk::AttachmentReference resolveAttachmentRef = {
      //       2,
      //       vk::ImageLayout::eColorAttachmentOptimal
      //    };

      vk::SubpassDescription subpass = {
         {}                               /*flags*/,
         vk::PipelineBindPoint::eGraphics /*pipelineBindPoint*/,
         0                                /*inputAttachmentCount*/,
         nullptr                          /*pInputAttachments*/,
         1                                /*colorAttachmentCount*/,
         &colorAttachmentRef              /*pColorAttachments*/,
         nullptr, //&resolveAttachmentRef /*pResolveAttachments*/,
         &depthAttachmentRef              /*pDepthStencilAttachment*/,
         0                                /*preserveAttachmentCount*/,
         nullptr                          /*pPreserveAttachments*/
      };

      std::vector<vk::SubpassDependency> dependencies = {
         {
            VK_SUBPASS_EXTERNAL                                                                    /*srcSubpass*/,
            0                                                                                      /*dstSubpass*/,
            vk::PipelineStageFlagBits::eColorAttachmentOutput                                      /*srcStageMask*/,
            vk::PipelineStageFlagBits::eColorAttachmentOutput                                      /*dstStageMask*/,
            vk::AccessFlags {}                                                                     /*srcAccessMask*/,
            vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite   /*dstAccessMask*/,
            vk::DependencyFlagBits::eByRegion                                                      /*dependencyFlags*/
         }//,
//          {
//             0                                                                                      /*srcSubpass*/,
//             VK_SUBPASS_EXTERNAL                                                                    /*dstSubpass*/,
//             vk::PipelineStageFlagBits::eColorAttachmentOutput                                      /*srcStageMask*/,
//             vk::PipelineStageFlagBits::eBottomOfPipe                                               /*dstStageMask*/,
//             vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite   /*srcAccessMask*/,
//             vk::AccessFlagBits::eMemoryRead                                                        /*dstAccessMask*/,
//             vk::DependencyFlagBits::eByRegion                                                      /*dependencyFlags*/
//          }
      };

      return m_Device->GetVkDevice().createRenderPass({
         {}                                         /*flags*/,
         static_cast<uint32_t>(attachments.size())  /*attachmentCount*/,
         attachments.data()                         /*pAttachments*/,
         1                                          /*subpassCount*/,
         &subpass                                   /*pSubpasses*/,
         static_cast<uint32_t>(dependencies.size()) /*dependencyCount*/,
         dependencies.data()                        /*pDependencies*/
      });
   }


   void VulkanGraphicsContext::DestroyRenderPass(vk::RenderPass renderPass) {
      if (m_Device) {
         if (renderPass) {
            m_Device->GetVkDevice().destroy(renderPass);
            renderPass = nullptr;
         }
      }
   }


   vk::DescriptorPool VulkanGraphicsContext::CreateDescriptorPool(const vk::ArrayProxy<const DescriptorBinding>& descriptorBindings, size_t maxSets) {
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


   void VulkanGraphicsContext::DestroyDescriptorPool(vk::DescriptorPool descriptorPool) {
      if (m_Device) {
         if (descriptorPool) {
            m_Device->GetVkDevice().destroy(descriptorPool);
            descriptorPool = nullptr;
         }
      }
   }


   void VulkanGraphicsContext::CreateCommandPool() {
      m_CommandPool = m_Device->GetVkDevice().createCommandPool({
         vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
         m_Device->GetGraphicsQueueFamilyIndex()
      });
   }


   void VulkanGraphicsContext::DestroyCommandPool() {
      if (m_Device && m_CommandPool) {
         m_Device->GetVkDevice().destroy(m_CommandPool);
         m_CommandPool = nullptr;
      }
   }


   void VulkanGraphicsContext::CreateCommandBuffers(const uint32_t commandBufferCount) {
      m_CommandBuffers = m_Device->GetVkDevice().allocateCommandBuffers({
         m_CommandPool                      /*commandPool*/,
         vk::CommandBufferLevel::ePrimary   /*level*/,
         commandBufferCount                 /*commandBufferCount*/
      });
   }


   void VulkanGraphicsContext::DestroyCommandBuffers() {
      if (m_Device && m_CommandPool) {
         m_Device->GetVkDevice().freeCommandBuffers(m_CommandPool, m_CommandBuffers);
         m_CommandBuffers.clear();
      }
   }


   void VulkanGraphicsContext::CreatePipelineCache() {
      m_PipelineCache = m_Device->GetVkDevice().createPipelineCache({});
   }


   void VulkanGraphicsContext::DestroyPipelineCache() {
      if (m_Device && m_PipelineCache) {
         m_Device->GetVkDevice().destroy(m_PipelineCache);
      }
   }


   VulkanWindowGC::VulkanWindowGC(std::shared_ptr<VulkanDevice> device, const Window& window)
   : VulkanGraphicsContext {device}
   , m_Window {static_cast<GLFWwindow*>(window.GetNativeWindow())}
   , m_IsVSync(window.IsVSync())
   {
      CreateSurface();
      CreateSwapChain();
      CreateImageViews();
      CreateDepthStencil();
      m_RenderPass = CreateRenderPass(true, true, vk::ImageLayout::ePresentSrcKHR);
      CreateFramebuffers();

      CreateCommandPool();
      CreateCommandBuffers(static_cast<uint32_t>(m_SwapChainImages.size()));
      CreateSyncObjects();
      CreatePipelineCache();

      EventDispatcher::Connect<WindowResizeEvent, &VulkanWindowGC::OnWindowResize>(*this);
      EventDispatcher::Connect<WindowVSyncChangedEvent, &VulkanWindowGC::OnWindowVSyncChanged>(*this);

      // Set clear values for all framebuffer attachments with loadOp set to clear
      // We use two attachments (color and depth) that are cleared at the start of the subpass and as such we need to set clear values for both
      glm::vec4 clearColor = window.GetClearColor();
      m_ClearValues = {
         vk::ClearColorValue {std::array<float, 4>{clearColor.r, clearColor.g, clearColor.b, clearColor.a}},
         vk::ClearDepthStencilValue {1.0f, 0}
      };

   }


   VulkanWindowGC::~VulkanWindowGC() {
      if (m_Device) {
         m_Device->GetVkDevice().waitIdle();
         if (ImGui::GetCurrentContext()) {
            ImGui_ImplVulkan_Shutdown();
            ImGui_ImplGlfw_Shutdown();
            ImGui::DestroyContext();
            DestroyRenderPass(m_RenderPassImGui);
            DestroyDescriptorPool(m_DescriptorPoolImGui);
         }
         DestroyPipelineCache();
         DestroySyncObjects();
         DestroyCommandBuffers();
         DestroyCommandPool();
         DestroyFramebuffers();
         DestroyRenderPass(m_RenderPass);
         DestroyDepthStencil();
         DestroyImageViews();
         DestroySwapChain(m_SwapChain);
         DestroySurface();
      }
   }


   void VulkanWindowGC::BeginFrame() {
      PKZL_PROFILE_FUNCTION();
      {
         PKZL_PROFILE_SCOPE("AquireNextImageKHR");
         auto rv = m_Device->GetVkDevice().acquireNextImageKHR(m_SwapChain, UINT64_MAX, m_ImageAvailableSemaphores[m_CurrentFrame], nullptr);

         if (rv.result == vk::Result::eErrorOutOfDateKHR) {
            RecreateSwapChain();
            return;
         } else if ((rv.result != vk::Result::eSuccess) && (rv.result != vk::Result::eSuboptimalKHR)) {
            throw std::runtime_error {"failed to acquire swap chain image!"};
         }

         m_CurrentImage = rv.value;
         PKZL_PROFILE_SETVALUE(m_CurrentImage);
      }

      // Wait until we know GPU has finished with the command buffer we are about to use...
      // Note that m_CurrentFrame and m_CurrentImage are not necessarily equal (particularly if we have, say, 3 swap chain images, and 2 frames-in-flight)
      // However, we know that the GPU has finished with m_CurrentImage'th command buffer so long as the m_CurrentFrame'th fence is signaled
      vk::Result result = m_Device->GetVkDevice().waitForFences(m_InFlightFences[m_CurrentFrame], true, UINT64_MAX);

      vk::CommandBufferBeginInfo commandBufferBI = {
         vk::CommandBufferUsageFlagBits::eSimultaneousUse
      };
      m_CommandBuffers[m_CurrentImage].begin(commandBufferBI);

      // TODO: Not sure that this is the best place to begin render pass.
      //       What if you need/want multiple render passes?  How will the client control this?

      vk::RenderPassBeginInfo renderPassBI = {
         m_RenderPass                                 /*renderPass*/,
         m_SwapChainFramebuffers[m_CurrentImage]      /*framebuffer*/,
         { {0,0}, m_Extent }                          /*renderArea*/,
         static_cast<uint32_t>(m_ClearValues.size())  /*clearValueCount*/,
         m_ClearValues.data()                         /*pClearValues*/
      };
      m_CommandBuffers[m_CurrentImage].beginRenderPass(renderPassBI, vk::SubpassContents::eInline);

      // Update dynamic state

      // At time of writing, VK_EXT_extended_dynamic_state is not available in the
      // nvidia general release drivers.
      // So, to change the frontface winding order (which we need to do so that we
      // face culling still gives correct result if/when we flip the viewport for
      // OpenGL/Vulkan interop of coordinate systems), we have two separate pipelines!
      //m_CommandBuffers.front().setFrontFaceEXT(vk::FrontFace::eClockwise);

      // Flip viewport. Pikzel uses 0,0 as bottom-left
      vk::Viewport viewportFlipped = {
         0.0f, static_cast<float>(m_Extent.height),
         static_cast<float>(m_Extent.width), -1.0f * static_cast<float>(m_Extent.height),
         0.0f, 1.0f
      };
      m_CommandBuffers[m_CurrentImage].setViewport(0, viewportFlipped);

      vk::Rect2D scissor = {
         {0, 0},
         m_Extent
      };
      m_CommandBuffers[m_CurrentImage].setScissor(0, scissor);
   }


   void VulkanWindowGC::EndFrame() {
      PKZL_PROFILE_FUNCTION();
      vk::CommandBuffer commandBuffer = m_CommandBuffers[m_CurrentImage];
      commandBuffer.endRenderPass();  // TODO: think about where render passes should begin/end

      if (m_ImGuiFrameStarted) {
         vk::RenderPassBeginInfo renderPassBI = {
            m_RenderPassImGui                          /*renderPass*/,
            m_SwapChainFramebuffers[m_CurrentImage]    /*framebuffer*/,
            { {0,0}, m_Extent }                        /*renderArea*/,
            0                                          /*clearValueCount*/,
            nullptr                                    /*pClearValues*/
         };

         commandBuffer.beginRenderPass(renderPassBI, vk::SubpassContents::eInline);
         ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);
         commandBuffer.endRenderPass();
      }

      commandBuffer.end();
      vk::PipelineStageFlags waitStages[] = {{vk::PipelineStageFlagBits::eColorAttachmentOutput}};
      vk::SubmitInfo si = {
         1                                             /*waitSemaphoreCount*/,
         &m_ImageAvailableSemaphores[m_CurrentFrame]   /*pWaitSemaphores*/,
         waitStages                                    /*pWaitDstStageMask*/,
         1                                             /*commandBufferCount*/,
         &commandBuffer                                /*pCommandBuffers*/,
         1                                             /*signalSemaphoreCount*/,
         &m_RenderFinishedSemaphores[m_CurrentFrame]   /*pSignalSemaphores*/
      };

      m_Device->GetVkDevice().resetFences(m_InFlightFences[m_CurrentFrame]);
      m_Device->GetGraphicsQueue().submit(si, m_InFlightFences[m_CurrentFrame]);

      if (m_ImGuiFrameStarted) {
         if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
         }
         m_ImGuiFrameStarted = false;
      }
   }


   void VulkanWindowGC::InitializeImGui() {
      IMGUI_CHECKVERSION();

      m_DescriptorPoolImGui = CreateDescriptorPool(DescriptorBinding {0, 1, vk::DescriptorType::eCombinedImageSampler, {}}, 10);
      m_RenderPassImGui = CreateRenderPass(false, false, vk::ImageLayout::ePresentSrcKHR);

      ImGui::CreateContext();

      ImGuiIO& io = ImGui::GetIO();
      io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
      io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
      io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

      ImGui_ImplGlfw_InitForVulkan(m_Window, true);
      ImGui_ImplVulkan_InitInfo init_info = {};
      init_info.Instance = m_Device->GetVkInstance();
      init_info.PhysicalDevice = m_Device->GetVkPhysicalDevice();
      init_info.Device = m_Device->GetVkDevice();
      init_info.QueueFamily = m_Device->GetGraphicsQueueFamilyIndex();
      init_info.Queue = m_Device->GetGraphicsQueue();
      init_info.PipelineCache = m_PipelineCache;
      init_info.DescriptorPool = m_DescriptorPoolImGui;
      init_info.Allocator = nullptr; // TODO: proper allocator...
      init_info.MinImageCount = static_cast<uint32_t>(m_SwapChainImages.size());
      init_info.ImageCount = static_cast<uint32_t>(m_SwapChainImages.size());
      init_info.CheckVkResultFn = [] (const VkResult err) {
         if (err != VK_SUCCESS) {
            throw std::runtime_error {"ImGui Vulkan error!"};
         }
      };
      ImGui_ImplVulkan_Init(&init_info, m_RenderPassImGui);
      __super::InitializeImGui();
   }


   void VulkanWindowGC::BeginImGuiFrame() {
      ImGui_ImplVulkan_NewFrame();
      ImGui_ImplGlfw_NewFrame();
      ImGui::NewFrame();
      m_ImGuiFrameStarted = true;
   }


   void VulkanWindowGC::EndImGuiFrame() {
      ImGui::Render();
   }


   void VulkanWindowGC::Bind(const Pipeline& pipeline) {
      const VulkanPipeline& vulkanPipeline = static_cast<const VulkanPipeline&>(pipeline);
      GetVkCommandBuffer().bindPipeline(vk::PipelineBindPoint::eGraphics, vulkanPipeline.GetVkPipelineFrontFaceCCW());
      m_Pipeline = const_cast<VulkanPipeline*>(&vulkanPipeline);
      m_Pipeline->UnbindDescriptorSets(); // *un*bind descriptor sets here.  This allows us to update the descriptors.  The descriptor sets are then bound just before we draw something (e.g. see DrawIndexed())
   }


   void VulkanWindowGC::Unbind(const Pipeline&) {
      m_Pipeline = nullptr;
   }


   void VulkanWindowGC::SwapBuffers() {
      PKZL_PROFILE_FUNCTION();

      vk::PresentInfoKHR piKHR = {
         1                                            /*waitSemaphoreCount*/,
         &m_RenderFinishedSemaphores[m_CurrentFrame]  /*pWaitSemaphores*/,
         1                                            /*swapchainCount*/,
         &m_SwapChain                                 /*pSwapchains*/,
         &m_CurrentImage                              /*pImageIndices*/,
         nullptr                                      /*pResults*/
      };

      // do not use cpp wrappers here.
      // The problem is that VK_ERROR_OUT_OF_DATE_KHR is an exception in the cpp wrapper, rather
      // than a valid return code.
      // Of course, you could try..catch but that seems quite an inefficient way to do it.
      //auto result = m_PresentQueue.presentKHR(pi);
      VkPresentInfoKHR vkPiKHR = (VkPresentInfoKHR)piKHR;
      auto result = vkQueuePresentKHR(m_Device->GetPresentQueue(), &vkPiKHR);
      if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || m_WantResize) {
         RecreateSwapChain();
      } else if (result != VK_SUCCESS) {
         throw std::runtime_error {"Failed to present swap chain image!"};
      }
      m_CurrentFrame = ++m_CurrentFrame % m_MaxFramesInFlight;
   }


   vk::CommandBuffer VulkanWindowGC::GetVkCommandBuffer() {
      return m_CommandBuffers[m_CurrentImage];
   }


   vk::Fence VulkanWindowGC::GetVkFence() {
      return m_InFlightFences[m_CurrentFrame];
   }

   void VulkanWindowGC::CreateSurface() {
      VkSurfaceKHR surface;
      if (glfwCreateWindowSurface(m_Device->GetVkInstance(), m_Window, nullptr, &surface) != VK_SUCCESS) {
         throw std::runtime_error {"failed to create window surface!"};
      }
      if (!m_Device->GetVkPhysicalDevice().getSurfaceSupportKHR(m_Device->GetPresentQueueFamilyIndex(), surface)) {
         throw std::runtime_error {"Vulkan physical device does not support window surface!"};
      }
      m_Surface = surface;
   }


   void VulkanWindowGC::DestroySurface() {
      if (m_Device && m_Device->GetVkInstance() && m_Surface) {
         m_Device->GetVkInstance().destroy(m_Surface);
         m_Surface = nullptr;
      }
   }


   vk::SurfaceFormatKHR VulkanWindowGC::SelectSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats) {
      for (const auto& availableFormat : availableFormats) {
         if (availableFormat.format == vk::Format::eB8G8R8A8Unorm && availableFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
            return availableFormat;
         }
      }
      return availableFormats[0];
   }


   vk::PresentModeKHR VulkanWindowGC::SelectPresentMode(const std::vector<vk::PresentModeKHR>& availablePresentModes) {
      if (!m_IsVSync) {
         for (const auto& availablePresentMode : availablePresentModes) {
            if (availablePresentMode == vk::PresentModeKHR::eMailbox) {
               return availablePresentMode;
            }
         }
      }
      return vk::PresentModeKHR::eFifo;
   }


   void VulkanWindowGC::CreateSwapChain() {
      vk::SwapchainKHR oldSwapChain = m_SwapChain;

      SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(m_Device->GetVkPhysicalDevice(), m_Surface);

      vk::SurfaceFormatKHR surfaceFormat = SelectSurfaceFormat(swapChainSupport.Formats);
      vk::PresentModeKHR presentMode = SelectPresentMode(swapChainSupport.PresentModes);
      m_Format = surfaceFormat.format;
      m_Extent = SelectSwapExtent(swapChainSupport.Capabilities);

      uint32_t imageCount = swapChainSupport.Capabilities.minImageCount;// +1;
      if ((swapChainSupport.Capabilities.maxImageCount > 0) && (imageCount > swapChainSupport.Capabilities.maxImageCount)) {
         imageCount = swapChainSupport.Capabilities.maxImageCount;
      }

      // Find the transformation of the surface
      vk::SurfaceTransformFlagBitsKHR preTransform;
      if (swapChainSupport.Capabilities.supportedTransforms & vk::SurfaceTransformFlagBitsKHR::eIdentity) {
         // We prefer a non-rotated transform
         preTransform = vk::SurfaceTransformFlagBitsKHR::eIdentity;
      } else {
         preTransform = swapChainSupport.Capabilities.currentTransform;
      }

      // Find a supported composite alpha format (not all devices support alpha opaque)
      vk::CompositeAlphaFlagBitsKHR compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
      // Select the first composite alpha format available
      std::array<vk::CompositeAlphaFlagBitsKHR, 4> compositeAlphaFlags = {
         vk::CompositeAlphaFlagBitsKHR::eOpaque,
         vk::CompositeAlphaFlagBitsKHR::ePreMultiplied,
         vk::CompositeAlphaFlagBitsKHR::ePostMultiplied,
         vk::CompositeAlphaFlagBitsKHR::eInherit
      };
      for (const auto& compositeAlphaFlag : compositeAlphaFlags) {
         if (swapChainSupport.Capabilities.supportedCompositeAlpha & compositeAlphaFlag) {
            compositeAlpha = compositeAlphaFlag;
            break;
         };
      }

      vk::SwapchainCreateInfoKHR ci;
      ci.surface = m_Surface;
      ci.minImageCount = imageCount;
      ci.imageFormat = m_Format;
      ci.imageColorSpace = surfaceFormat.colorSpace;
      ci.imageExtent = m_Extent;
      ci.imageArrayLayers = 1;
      ci.imageUsage = vk::ImageUsageFlagBits::eColorAttachment;
      ci.preTransform = preTransform;
      ci.compositeAlpha = compositeAlpha;
      ci.presentMode = presentMode;
      ci.clipped = true;
      ci.oldSwapchain = oldSwapChain;

      uint32_t queueFamilyIndices[] = {m_Device->GetGraphicsQueueFamilyIndex(), m_Device->GetPresentQueueFamilyIndex()};
      if (m_Device->GetGraphicsQueueFamilyIndex() != m_Device->GetPresentQueueFamilyIndex()) {
         ci.imageSharingMode = vk::SharingMode::eConcurrent;
         ci.queueFamilyIndexCount = 2;
         ci.pQueueFamilyIndices = queueFamilyIndices;
      }

      // Enable transfer source on swap chain images if supported
      if (swapChainSupport.Capabilities.supportedUsageFlags & vk::ImageUsageFlagBits::eTransferSrc) {
         ci.imageUsage |= vk::ImageUsageFlagBits::eTransferSrc;
      }

      // Enable transfer destination on swap chain images if supported
      if (swapChainSupport.Capabilities.supportedUsageFlags & vk::ImageUsageFlagBits::eTransferDst) {
         ci.imageUsage |= vk::ImageUsageFlagBits::eTransferDst;
      }

      m_SwapChain = m_Device->GetVkDevice().createSwapchainKHR(ci);
      DestroySwapChain(oldSwapChain);
      std::vector<vk::Image> swapChainImages = m_Device->GetVkDevice().getSwapchainImagesKHR(m_SwapChain);
      for (const auto& image : swapChainImages) {
         m_SwapChainImages.emplace_back(m_Device, image, m_Format, m_Extent);
      }
   }


   void VulkanWindowGC::DestroySwapChain(vk::SwapchainKHR& swapChain) {
      if (m_Device && swapChain) {
         m_SwapChainImages.clear();
         m_Device->GetVkDevice().destroy(swapChain);
         swapChain = nullptr;
      }
   }


   void VulkanWindowGC::CreateImageViews() {
      for (auto& image : m_SwapChainImages) {
         image.CreateImageView(m_Format, vk::ImageAspectFlagBits::eColor);
      }
   }


   void VulkanWindowGC::DestroyImageViews() {
      if (m_Device) {
         for (auto& image : m_SwapChainImages) {
            image.DestroyImageView();
         }
      }
   }


   vk::Extent2D VulkanWindowGC::SelectSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities) {
      if (capabilities.currentExtent.width != UINT32_MAX) {
         return capabilities.currentExtent;
      } else {
         int width;
         int height;
         glfwGetFramebufferSize(m_Window, &width, &height);
         vk::Extent2D actualExtent = {
               static_cast<uint32_t>(width),
               static_cast<uint32_t>(height)
         };
         actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
         actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

         return actualExtent;
      }
   }


   void VulkanWindowGC::CreateFramebuffers() {
      std::array<vk::ImageView, 2> attachments = {
         nullptr,
         m_DepthImage->GetVkImageView()
      };
      vk::FramebufferCreateInfo ci = {
         {}                                        /*flags*/,
         m_RenderPass                              /*renderPass*/,
         static_cast<uint32_t>(attachments.size()) /*attachmentCount*/,
         attachments.data()                        /*pAttachments*/,
         m_Extent.width                            /*width*/,
         m_Extent.height                           /*height*/,
         1                                         /*layers*/
      };

      m_SwapChainFramebuffers.reserve(m_SwapChainImages.size());
      for (const auto& swapChainImage : m_SwapChainImages) {
         attachments[0] = swapChainImage.GetVkImageView();
         m_SwapChainFramebuffers.push_back(m_Device->GetVkDevice().createFramebuffer(ci));
      }
   }


   void VulkanWindowGC::DestroyFramebuffers() {
      if (m_Device) {
         for (auto framebuffer : m_SwapChainFramebuffers) {
            m_Device->GetVkDevice().destroy(framebuffer);
         }
         m_SwapChainFramebuffers.clear();
      }
   }


   void VulkanWindowGC::CreateSyncObjects() {
      m_ImageAvailableSemaphores.reserve(m_MaxFramesInFlight);
      m_RenderFinishedSemaphores.reserve(m_MaxFramesInFlight);
      m_InFlightFences.reserve(m_MaxFramesInFlight);

      vk::FenceCreateInfo ci = {
         {vk::FenceCreateFlagBits::eSignaled}
      };

      for (uint32_t i = 0; i < m_MaxFramesInFlight; ++i) {
         m_ImageAvailableSemaphores.emplace_back(m_Device->GetVkDevice().createSemaphore({}));
         m_RenderFinishedSemaphores.emplace_back(m_Device->GetVkDevice().createSemaphore({}));
         m_InFlightFences.emplace_back(m_Device->GetVkDevice().createFence(ci));
      }
   }


   void VulkanWindowGC::DestroySyncObjects() {
      if (m_Device) {
         for (auto semaphore : m_ImageAvailableSemaphores) {
            m_Device->GetVkDevice().destroy(semaphore);
         }
         m_ImageAvailableSemaphores.clear();

         for (auto semaphore : m_RenderFinishedSemaphores) {
            m_Device->GetVkDevice().destroy(semaphore);
         }
         m_RenderFinishedSemaphores.clear();

         for (auto fence : m_InFlightFences) {
            m_Device->GetVkDevice().destroy(fence);
         }
         m_InFlightFences.clear();
      }
   }


   void VulkanWindowGC::RecreateSwapChain() {
      m_Device->GetVkDevice().waitIdle();
      DestroyImageViews();
      CreateSwapChain();
      CreateImageViews();

      DestroyDepthStencil();
      CreateDepthStencil();

      DestroyFramebuffers();
      CreateFramebuffers();

      m_WantResize = false;
   }


   void VulkanWindowGC::OnWindowResize(const WindowResizeEvent& event) {
      if (event.Sender == m_Window) {
         m_WantResize = true;
      }
   }


   void VulkanWindowGC::OnWindowVSyncChanged(const WindowVSyncChangedEvent& event) {
      if (event.Sender == m_Window) {
         m_IsVSync = event.IsVSync;
         m_WantResize = true; // force swapchain recreate
      }
   }


   VulkanFramebufferGC::VulkanFramebufferGC(std::shared_ptr<VulkanDevice> device, VulkanFramebuffer* framebuffer)
   : VulkanGraphicsContext {device}
   , m_Framebuffer {framebuffer}
   {
      // Set clear values for all framebuffer attachments with loadOp set to clear
      // We use two attachments (color and depth) that are cleared at the start of the subpass and as such we need to set clear values for both
      glm::vec4 clearColor = framebuffer->GetClearColor();
      m_ClearValues = {
         vk::ClearColorValue {std::array<float, 4>{clearColor.r, clearColor.g, clearColor.b, clearColor.a}},
         vk::ClearDepthStencilValue {1.0f, 0}
      };
      m_Format = m_Framebuffer->GetVkFormat();
      m_DepthFormat = m_Framebuffer->GetVkDepthFormat();
      m_Extent = vk::Extent2D{m_Framebuffer->GetWidth(), m_Framebuffer->GetHeight()};
      m_RenderPass = CreateRenderPass(true, true, vk::ImageLayout::eShaderReadOnlyOptimal);
      CreateCommandPool();
      CreateCommandBuffers(1);
      CreateSyncObjects();
      CreatePipelineCache();
   }


   VulkanFramebufferGC::~VulkanFramebufferGC() {
      if (m_Device) {
         m_Device->GetVkDevice().waitIdle();

         if (m_Pipeline) {
            Unbind(*m_Pipeline);
         }
         DestroyPipelineCache();
         DestroySyncObjects();
         DestroyCommandBuffers();
         DestroyCommandPool();
         DestroyRenderPass(m_RenderPass);
      }
   }


   void VulkanFramebufferGC::BeginFrame() {
      PKZL_PROFILE_FUNCTION();

      m_CommandBuffers.front().begin({
         vk::CommandBufferUsageFlagBits::eSimultaneousUse
      });

      // TODO: Not sure that this is the best place to begin render pass.
      //       What if you need/want multiple render passes?  How will the client control this?

      vk::RenderPassBeginInfo renderPassBI = {
         m_RenderPass                                 /*renderPass*/,
         m_Framebuffer->GetVkFramebuffer()            /*framebuffer*/,
         { {0,0}, m_Extent }                          /*renderArea*/,
         static_cast<uint32_t>(m_ClearValues.size())  /*clearValueCount*/,
         m_ClearValues.data()                         /*pClearValues*/
      };
      m_CommandBuffers.front().beginRenderPass(renderPassBI, vk::SubpassContents::eInline);

      // Update dynamic state:

      // At time of writing, VK_EXT_extended_dynamic_state is not available in the
      // nvidia general release drivers.
      // So, to change the frontface winding order (which we need to do so that we
      // face culling still gives correct result if/when we flip the viewport for
      // OpenGL/Vulkan interop of coordinate systems), we have two separate pipelines!
      //m_CommandBuffers.front().setFrontFaceEXT(vk::FrontFace::eClockwise);

      // Do not flip viewport here.  Means FBO color attachment can then be used directly as a "texture" in later
      // rendering operations.  Remember Pikzel has 0,0 as bottom-left
      vk::Viewport viewport = {
         0.0f, 0.0f,
         static_cast<float>(m_Extent.width), static_cast<float>(m_Extent.height),
         0.0f, 1.0f
      };
      m_CommandBuffers.front().setViewport(0, viewport);

      vk::Rect2D scissor = {
         {0, 0},
         m_Extent
      };
      m_CommandBuffers.front().setScissor(0, scissor);
   }


   void VulkanFramebufferGC::EndFrame() {
      PKZL_PROFILE_FUNCTION();
      vk::CommandBuffer commandBuffer = m_CommandBuffers.front();
      commandBuffer.endRenderPass();  // TODO: think about where render passes should begin/end
      commandBuffer.end();

      vk::PipelineStageFlags waitStages[] = {{vk::PipelineStageFlagBits::eColorAttachmentOutput}};
      vk::SubmitInfo si = {
         0                /*waitSemaphoreCount*/,
         nullptr          /*pWaitSemaphores*/,
         waitStages       /*pWaitDstStageMask*/,
         1                /*commandBufferCount*/,
         &commandBuffer   /*pCommandBuffers*/,
         0                /*signalSemaphoreCount*/,
         nullptr          /*pSignalSemaphores*/
      };

      m_Device->GetVkDevice().resetFences(m_InFlightFence);
      m_Device->GetGraphicsQueue().submit(si, m_InFlightFence);
   }


   void VulkanFramebufferGC::Bind(const Pipeline& pipeline) {
      const VulkanPipeline& vulkanPipeline = static_cast<const VulkanPipeline&>(pipeline);
      GetVkCommandBuffer().bindPipeline(vk::PipelineBindPoint::eGraphics, vulkanPipeline.GetVkPipelineFrontFaceCW());
      m_Pipeline = const_cast<VulkanPipeline*>(&vulkanPipeline);
      m_Pipeline->UnbindDescriptorSets();
   }


   void VulkanFramebufferGC::Unbind(const Pipeline&) {
      m_Pipeline->UnbindDescriptorSets();
      m_Pipeline = nullptr;
   }


   void VulkanFramebufferGC::SwapBuffers() {
      vk::Result result = m_Device->GetVkDevice().waitForFences(m_InFlightFence, true, UINT64_MAX);
   }


   vk::CommandBuffer VulkanFramebufferGC::GetVkCommandBuffer() {
      return m_CommandBuffers.front();
   }


   vk::Fence VulkanFramebufferGC::GetVkFence() {
      return m_InFlightFence;

   }


   void VulkanFramebufferGC::CreateSyncObjects() {
      m_InFlightFence = m_Device->GetVkDevice().createFence({
         {vk::FenceCreateFlagBits::eSignaled}
      });
   }


   void VulkanFramebufferGC::DestroySyncObjects() {
      if (m_Device && m_InFlightFence) {
         m_Device->GetVkDevice().destroy(m_InFlightFence);
         m_InFlightFence = nullptr;
      }
   }

}
