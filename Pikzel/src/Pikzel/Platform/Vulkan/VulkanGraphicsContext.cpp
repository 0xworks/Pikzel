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
      super::InitializeImGui();
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


   void VulkanGraphicsContext::Bind(const Id resourceId, const UniformBuffer& buffer) {
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


   void VulkanGraphicsContext::Unbind(const UniformBuffer&) {}


   void VulkanGraphicsContext::Bind(const Id resourceId, const Texture& texture) {
      const VulkanResource& resource = m_Pipeline->GetResource(resourceId);

      vk::Sampler sampler = static_cast<const VulkanTexture&>(texture).GetVkSampler();
      vk::ImageView imageView = static_cast<const VulkanTexture&>(texture).GetVkImageView();
      vk::DescriptorImageInfo textureImageDescriptor = {
         sampler,
         imageView,
         vk::ImageLayout::eShaderReadOnlyOptimal
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


   void VulkanGraphicsContext::Unbind(const Texture&) {}


   std::unique_ptr<Pikzel::Pipeline> VulkanGraphicsContext::CreatePipeline(const PipelineSettings& settings) const {
      return std::make_unique<VulkanPipeline>(m_Device, *this, settings);
   }


   void VulkanGraphicsContext::PushConstant(const Id id, bool value) {
      PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
      const VulkanPushConstant& constant = m_Pipeline->GetPushConstant(id);
      PKZL_CORE_ASSERT(constant.Type == DataType::Bool, "Push constant '{0}' type mismatch.  Bool given, expected {1}!", constant.Name, DataTypeToString(constant.Type));
      GetVkCommandBuffer().pushConstants<bool>(m_Pipeline->GetVkPipelineLayout(), constant.ShaderStages, constant.Offset, value);
   }


   void VulkanGraphicsContext::PushConstant(const Id id, int value) {
      PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
      const VulkanPushConstant& constant = m_Pipeline->GetPushConstant(id);
      PKZL_CORE_ASSERT(constant.Type == DataType::Int, "Push constant '{0}' type mismatch.  Int given, expected {1}!", constant.Name, DataTypeToString(constant.Type));
      GetVkCommandBuffer().pushConstants<int>(m_Pipeline->GetVkPipelineLayout(), constant.ShaderStages, constant.Offset, value);
   }


   void VulkanGraphicsContext::PushConstant(const Id id, uint32_t value) {
      PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
      const VulkanPushConstant& constant = m_Pipeline->GetPushConstant(id);
      PKZL_CORE_ASSERT(constant.Type == DataType::UInt, "Push constant '{0}' type mismatch.  UInt given, expected {1}!", constant.Name, DataTypeToString(constant.Type));
      GetVkCommandBuffer().pushConstants<uint32_t>(m_Pipeline->GetVkPipelineLayout(), constant.ShaderStages, constant.Offset, value);
   }


   void VulkanGraphicsContext::PushConstant(const Id id, float value) {
      PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
      const VulkanPushConstant& constant = m_Pipeline->GetPushConstant(id);
      PKZL_CORE_ASSERT(constant.Type == DataType::Float, "Push constant '{0}' type mismatch.  Float given, expected {1}!", constant.Name, DataTypeToString(constant.Type));
      GetVkCommandBuffer().pushConstants<float>(m_Pipeline->GetVkPipelineLayout(), constant.ShaderStages, constant.Offset, value);
   }


   void VulkanGraphicsContext::PushConstant(const Id id, double value) {
      PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
      const VulkanPushConstant& constant = m_Pipeline->GetPushConstant(id);
      PKZL_CORE_ASSERT(constant.Type == DataType::Double, "Push constant '{0}' type mismatch.  Double given, expected {1}!", constant.Name, DataTypeToString(constant.Type));
      GetVkCommandBuffer().pushConstants<double>(m_Pipeline->GetVkPipelineLayout(), constant.ShaderStages, constant.Offset, value);
   }


   void VulkanGraphicsContext::PushConstant(const Id id, const glm::bvec2& value) {
      PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
      const VulkanPushConstant& constant = m_Pipeline->GetPushConstant(id);
      PKZL_CORE_ASSERT(constant.Type == DataType::BVec2, "Push constant '{0}' type mismatch.  BVec2 given, expected {1}!", constant.Name, DataTypeToString(constant.Type));
      GetVkCommandBuffer().pushConstants<glm::bvec2>(m_Pipeline->GetVkPipelineLayout(), constant.ShaderStages, constant.Offset, value);
   }


   void VulkanGraphicsContext::PushConstant(const Id id, const glm::bvec3& value) {
      PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
      const VulkanPushConstant& constant = m_Pipeline->GetPushConstant(id);
      PKZL_CORE_ASSERT(constant.Type == DataType::BVec3, "Push constant '{0}' type mismatch.  BVec3 given, expected {1}!", constant.Name, DataTypeToString(constant.Type));
      GetVkCommandBuffer().pushConstants<glm::bvec3>(m_Pipeline->GetVkPipelineLayout(), constant.ShaderStages, constant.Offset, value);
   }


   void VulkanGraphicsContext::PushConstant(const Id id, const glm::bvec4& value) {
      PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
      const VulkanPushConstant& constant = m_Pipeline->GetPushConstant(id);
      PKZL_CORE_ASSERT(constant.Type == DataType::BVec4, "Push constant '{0}' type mismatch.  BVec4 given, expected {1}!", constant.Name, DataTypeToString(constant.Type));
      GetVkCommandBuffer().pushConstants<glm::bvec4>(m_Pipeline->GetVkPipelineLayout(), constant.ShaderStages, constant.Offset, value);
   }


   void VulkanGraphicsContext::PushConstant(const Id id, const glm::ivec2& value) {
      PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
      const VulkanPushConstant& constant = m_Pipeline->GetPushConstant(id);
      PKZL_CORE_ASSERT(constant.Type == DataType::IVec2, "Push constant '{0}' type mismatch.  IVec2 given, expected {1}!", constant.Name, DataTypeToString(constant.Type));
      GetVkCommandBuffer().pushConstants<glm::ivec2>(m_Pipeline->GetVkPipelineLayout(), constant.ShaderStages, constant.Offset, value);
   }


   void VulkanGraphicsContext::PushConstant(const Id id, const glm::ivec3& value) {
      PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
      const VulkanPushConstant& constant = m_Pipeline->GetPushConstant(id);
      PKZL_CORE_ASSERT(constant.Type == DataType::IVec3, "Push constant '{0}' type mismatch.  IVec3 given, expected {1}!", constant.Name, DataTypeToString(constant.Type));
      GetVkCommandBuffer().pushConstants<glm::ivec3>(m_Pipeline->GetVkPipelineLayout(), constant.ShaderStages, constant.Offset, value);
   }


   void VulkanGraphicsContext::PushConstant(const Id id, const glm::ivec4& value) {
      PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
      const VulkanPushConstant& constant = m_Pipeline->GetPushConstant(id);
      PKZL_CORE_ASSERT(constant.Type == DataType::IVec4, "Push constant '{0}' type mismatch.  IVec4 given, expected {1}!", constant.Name, DataTypeToString(constant.Type));
      GetVkCommandBuffer().pushConstants<glm::ivec4>(m_Pipeline->GetVkPipelineLayout(), constant.ShaderStages, constant.Offset, value);
   }


   void VulkanGraphicsContext::PushConstant(const Id id, const glm::uvec2& value) {
      PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
      const VulkanPushConstant& constant = m_Pipeline->GetPushConstant(id);
      PKZL_CORE_ASSERT(constant.Type == DataType::UVec2, "Push constant '{0}' type mismatch.  UVec2 given, expected {1}!", constant.Name, DataTypeToString(constant.Type));
      GetVkCommandBuffer().pushConstants<glm::uvec2>(m_Pipeline->GetVkPipelineLayout(), constant.ShaderStages, constant.Offset, value);
   }


   void VulkanGraphicsContext::PushConstant(const Id id, const glm::uvec3& value) {
      PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
      const VulkanPushConstant& constant = m_Pipeline->GetPushConstant(id);
      PKZL_CORE_ASSERT(constant.Type == DataType::UVec3, "Push constant '{0}' type mismatch.  UVec3 given, expected {1}!", constant.Name, DataTypeToString(constant.Type));
      GetVkCommandBuffer().pushConstants<glm::uvec3>(m_Pipeline->GetVkPipelineLayout(), constant.ShaderStages, constant.Offset, value);
   }


   void VulkanGraphicsContext::PushConstant(const Id id, const glm::uvec4& value) {
      PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
      const VulkanPushConstant& constant = m_Pipeline->GetPushConstant(id);
      PKZL_CORE_ASSERT(constant.Type == DataType::UVec4, "Push constant '{0}' type mismatch.  UVec4 given, expected {1}!", constant.Name, DataTypeToString(constant.Type));
      GetVkCommandBuffer().pushConstants<glm::uvec4>(m_Pipeline->GetVkPipelineLayout(), constant.ShaderStages, constant.Offset, value);
   }


   void VulkanGraphicsContext::PushConstant(const Id id, const glm::vec2& value) {
      PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
      const VulkanPushConstant& constant = m_Pipeline->GetPushConstant(id);
      PKZL_CORE_ASSERT(constant.Type == DataType::Vec2, "Push constant '{0}' type mismatch.  Vec2 given, expected {1}!", constant.Name, DataTypeToString(constant.Type));
      GetVkCommandBuffer().pushConstants<glm::vec2>(m_Pipeline->GetVkPipelineLayout(), constant.ShaderStages, constant.Offset, value);
   }


   void VulkanGraphicsContext::PushConstant(const Id id, const glm::vec3& value) {
      PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
      const VulkanPushConstant& constant = m_Pipeline->GetPushConstant(id);
      PKZL_CORE_ASSERT(constant.Type == DataType::Vec3, "Push constant '{0}' type mismatch.  Vec3 given, expected {1}!", constant.Name, DataTypeToString(constant.Type));
      GetVkCommandBuffer().pushConstants<glm::vec3>(m_Pipeline->GetVkPipelineLayout(), constant.ShaderStages, constant.Offset, value);
   }


   void VulkanGraphicsContext::PushConstant(const Id id, const glm::vec4& value) {
      PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
      const VulkanPushConstant& constant = m_Pipeline->GetPushConstant(id); // name will be like constants.mvp,  or anotherConstants.something4
      PKZL_CORE_ASSERT(constant.Type == DataType::Vec4, "Push constant '{0}' type mismatch.  Vec4 given, expected {1}!", constant.Name, DataTypeToString(constant.Type));
      GetVkCommandBuffer().pushConstants<glm::vec4>(m_Pipeline->GetVkPipelineLayout(), constant.ShaderStages, constant.Offset, value);
   }


   void VulkanGraphicsContext::PushConstant(const Id id, const glm::dvec2& value) {
      PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
      const VulkanPushConstant& constant = m_Pipeline->GetPushConstant(id);
      PKZL_CORE_ASSERT(constant.Type == DataType::DVec2, "Push constant '{0}' type mismatch.  DVec2 given, expected {1}!", constant.Name, DataTypeToString(constant.Type));
      GetVkCommandBuffer().pushConstants<glm::dvec2>(m_Pipeline->GetVkPipelineLayout(), constant.ShaderStages, constant.Offset, value);
   }


   void VulkanGraphicsContext::PushConstant(const Id id, const glm::dvec3& value) {
      PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
      const VulkanPushConstant& constant = m_Pipeline->GetPushConstant(id);
      PKZL_CORE_ASSERT(constant.Type == DataType::DVec3, "Push constant '{0}' type mismatch.  DVec3 given, expected {1}!", constant.Name, DataTypeToString(constant.Type));
      GetVkCommandBuffer().pushConstants<glm::dvec3>(m_Pipeline->GetVkPipelineLayout(), constant.ShaderStages, constant.Offset, value);
   }


   void VulkanGraphicsContext::PushConstant(const Id id, const glm::dvec4& value) {
      PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
      const VulkanPushConstant& constant = m_Pipeline->GetPushConstant(id);
      PKZL_CORE_ASSERT(constant.Type == DataType::DVec4, "Push constant '{0}' type mismatch.  DVec4 given, expected {1}!", constant.Name, DataTypeToString(constant.Type));
      GetVkCommandBuffer().pushConstants<glm::dvec4>(m_Pipeline->GetVkPipelineLayout(), constant.ShaderStages, constant.Offset, value);
   }


   void VulkanGraphicsContext::PushConstant(const Id id, const glm::mat2& value) {
      PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
      const VulkanPushConstant& constant = m_Pipeline->GetPushConstant(id);
      PKZL_CORE_ASSERT(constant.Type == DataType::Mat2, "Push constant '{0}' type mismatch.  Mat2 given, expected {1}!", constant.Name, DataTypeToString(constant.Type));
      GetVkCommandBuffer().pushConstants<glm::mat2>(m_Pipeline->GetVkPipelineLayout(), constant.ShaderStages, constant.Offset, value);
   }


   //void VulkanGraphicsContext::PushConstant(const Id id, const glm::mat2x3& value) {
   //   PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
   //   const VulkanPushConstant& constant = m_Pipeline->GetPushConstant(id);
   //   PKZL_CORE_ASSERT(constant.Type == DataType::Mat2x3, "Push constant '{0}' type mismatch.  Mat2x3 given, expected {1}!", constant.Name, DataTypeToString(constant.Type));
   //   GetVkCommandBuffer().pushConstants<glm::mat2x3>(m_Pipeline->GetVkPipelineLayout(), constant.ShaderStages, constant.Offset, value);
   //}


   void VulkanGraphicsContext::PushConstant(const Id id, const glm::mat2x4& value) {
      PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
      const VulkanPushConstant& constant = m_Pipeline->GetPushConstant(id);
      PKZL_CORE_ASSERT(constant.Type == DataType::Mat2x4, "Push constant '{0}' type mismatch.  Mat2x4 given, expected {1}!", constant.Name, DataTypeToString(constant.Type));
      GetVkCommandBuffer().pushConstants<glm::mat2x4>(m_Pipeline->GetVkPipelineLayout(), constant.ShaderStages, constant.Offset, value);
   }


   void VulkanGraphicsContext::PushConstant(const Id id, const glm::mat3x2& value) {
      PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
      const VulkanPushConstant& constant = m_Pipeline->GetPushConstant(id);
      PKZL_CORE_ASSERT(constant.Type == DataType::Mat3x2, "Push constant '{0}' type mismatch.  Mat3x2 given, expected {1}!", constant.Name, DataTypeToString(constant.Type));
      GetVkCommandBuffer().pushConstants<glm::mat3x2>(m_Pipeline->GetVkPipelineLayout(), constant.ShaderStages, constant.Offset, value);
   }


   //void VulkanGraphicsContext::PushConstant(const Id id, const glm::mat3& value) {
   //   PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
   //   const VulkanPushConstant& constant = m_Pipeline->GetPushConstant(id);
   //   PKZL_CORE_ASSERT(constant.Type == DataType::Mat3, "Push constant '{0}' type mismatch.  Mat3 given, expected {1}!", constant.Name, DataTypeToString(constant.Type));
   //   GetVkCommandBuffer().pushConstants<glm::mat3>(m_Pipeline->GetVkPipelineLayout(), constant.ShaderStages, constant.Offset, value);
   //}


   void VulkanGraphicsContext::PushConstant(const Id id, const glm::mat3x4& value) {
      PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
      const VulkanPushConstant& constant = m_Pipeline->GetPushConstant(id);
      PKZL_CORE_ASSERT(constant.Type == DataType::Mat3x4, "Push constant '{0}' type mismatch.  Mat3x4 given, expected {1}!", constant.Name, DataTypeToString(constant.Type));
      GetVkCommandBuffer().pushConstants<glm::mat3x4>(m_Pipeline->GetVkPipelineLayout(), constant.ShaderStages, constant.Offset, value);
   }


   void VulkanGraphicsContext::PushConstant(const Id id, const glm::mat4x2& value) {
      PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
      const VulkanPushConstant& constant = m_Pipeline->GetPushConstant(id);
      PKZL_CORE_ASSERT(constant.Type == DataType::Mat4x2, "Push constant '{0}' type mismatch.  Mat4x2 given, expected {1}!", constant.Name, DataTypeToString(constant.Type));
      GetVkCommandBuffer().pushConstants<glm::mat4x2>(m_Pipeline->GetVkPipelineLayout(), constant.ShaderStages, constant.Offset, value);
   }


   //void VulkanGraphicsContext::PushConstant(const Id id, const glm::mat4x3& value) {
   //   PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
   //   const VulkanPushConstant& constant = m_Pipeline->GetPushConstant(id);
   //   PKZL_CORE_ASSERT(constant.Type == DataType::Mat4x3, "Push constant '{0}' type mismatch.  Mat4x3 given, expected {1}!", constant.Name, DataTypeToString(constant.Type));
   //   m_CommandBuffers[m_CurrentImage].pushConstants<glm::mat4x3>(m_Pipeline->GetVkPipelineLayout(), constant.ShaderStages, constant.Offset, value);
   //}


   void VulkanGraphicsContext::PushConstant(const Id id, const glm::mat4& value) {
      PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
      const VulkanPushConstant& constant = m_Pipeline->GetPushConstant(id); // name will be like constants.mvp,  or anotherConstants.something4
      PKZL_CORE_ASSERT(constant.Type == DataType::Mat4, "Push constant '{0}' type mismatch.  Mat4 given, expected {1}!", constant.Name, DataTypeToString(constant.Type));
      GetVkCommandBuffer().pushConstants<glm::mat4>(m_Pipeline->GetVkPipelineLayout(), constant.ShaderStages, constant.Offset, value);
   }


   void VulkanGraphicsContext::PushConstant(const Id id, const glm::dmat2& value) {
      PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
      const VulkanPushConstant& constant = m_Pipeline->GetPushConstant(id);
      PKZL_CORE_ASSERT(constant.Type == DataType::DMat2, "Push constant '{0}' type mismatch.  DMat2 given, expected {1}!", constant.Name, DataTypeToString(constant.Type));
      GetVkCommandBuffer().pushConstants<glm::dmat2>(m_Pipeline->GetVkPipelineLayout(), constant.ShaderStages, constant.Offset, value);
   }


   //void VulkanGraphicsContext::PushConstant(const Id id, const glm::dmat2x3& value) {
   //   PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
   //   const VulkanPushConstant& constant = m_Pipeline->GetPushConstant(id);
   //   PKZL_CORE_ASSERT(constant.Type == DataType::DMat2x3, "Push constant '{0}' type mismatch.  DMat2x3 given, expected {1}!", constant.Name, DataTypeToString(constant.Type));
   //   m_CommandBuffers[m_CurrentImage].pushConstants<glm::dmat2x3>(m_Pipeline->GetVkPipelineLayout(), constant.ShaderStages, constant.Offset, value);
   //}


   void VulkanGraphicsContext::PushConstant(const Id id, const glm::dmat2x4& value) {
      PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
      const VulkanPushConstant& constant = m_Pipeline->GetPushConstant(id);
      PKZL_CORE_ASSERT(constant.Type == DataType::DMat2x4, "Push constant '{0}' type mismatch.  DMat2x4 given, expected {1}!", constant.Name, DataTypeToString(constant.Type));
      GetVkCommandBuffer().pushConstants<glm::dmat2x4>(m_Pipeline->GetVkPipelineLayout(), constant.ShaderStages, constant.Offset, value);
   }


   void VulkanGraphicsContext::PushConstant(const Id id, const glm::dmat3x2& value) {
      PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
      const VulkanPushConstant& constant = m_Pipeline->GetPushConstant(id);
      PKZL_CORE_ASSERT(constant.Type == DataType::DMat3x2, "Push constant '{0}' type mismatch.  DMat3x2 given, expected {1}!", constant.Name, DataTypeToString(constant.Type));
      GetVkCommandBuffer().pushConstants<glm::dmat3x2>(m_Pipeline->GetVkPipelineLayout(), constant.ShaderStages, constant.Offset, value);
   }


   //void VulkanGraphicsContext::PushConstant(const Id id, const glm::dmat3& value) {
   //   PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
   //   const VulkanPushConstant& constant = m_Pipeline->GetPushConstant(id);
   //   PKZL_CORE_ASSERT(constant.Type == DataType::DMat3, "Push constant '{0}' type mismatch.  DMat3 given, expected {1}!", constant.Name, DataTypeToString(constant.Type));
   //   GetVkCommandBuffer().pushConstants<glm::dmat3>(m_Pipeline->GetVkPipelineLayout(), constant.ShaderStages, constant.Offset, value);
   //}


   void VulkanGraphicsContext::PushConstant(const Id id, const glm::dmat3x4& value) {
      PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
      const VulkanPushConstant& constant = m_Pipeline->GetPushConstant(id);
      PKZL_CORE_ASSERT(constant.Type == DataType::DMat3x4, "Push constant '{0}' type mismatch.  DMat3x4 given, expected {1}!", constant.Name, DataTypeToString(constant.Type));
      GetVkCommandBuffer().pushConstants<glm::dmat3x4>(m_Pipeline->GetVkPipelineLayout(), constant.ShaderStages, constant.Offset, value);
   }


   void VulkanGraphicsContext::PushConstant(const Id id, const glm::dmat4x2& value) {
      PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
      const VulkanPushConstant& constant = m_Pipeline->GetPushConstant(id);
      PKZL_CORE_ASSERT(constant.Type == DataType::DMat4x2, "Push constant '{0}' type mismatch.  DMat4x2 given, expected {1}!", constant.Name, DataTypeToString(constant.Type));
      GetVkCommandBuffer().pushConstants<glm::dmat4x2>(m_Pipeline->GetVkPipelineLayout(), constant.ShaderStages, constant.Offset, value);
   }


   //void VulkanGraphicsContext::PushConstant(const Id id, const glm::dmat4x3& value) {
   //   PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
   //   const VulkanPushConstant& constant = m_Pipeline->GetPushConstant(id);
   //   PKZL_CORE_ASSERT(constant.Type == DataType::DMat4x3, "Push constant '{0}' type mismatch.  DMat4x3 given, expected {1}!", constant.Name, DataTypeToString(constant.Type));
   //   GetVkCommandBuffer().pushConstants<glm::dmat4x3>(m_Pipeline->GetVkPipelineLayout(), constant.ShaderStages, constant.Offset, value);
   //}


   void VulkanGraphicsContext::PushConstant(const Id id, const glm::dmat4& value) {
      PKZL_CORE_ASSERT(m_Pipeline, "Attempted to access null pipeline!");
      const VulkanPushConstant& constant = m_Pipeline->GetPushConstant(id);
      PKZL_CORE_ASSERT(constant.Type == DataType::DMat4, "Push constant '{0}' type mismatch.  DMat4 given, expected {1}!", constant.Name, DataTypeToString(constant.Type));
      GetVkCommandBuffer().pushConstants<glm::dmat4>(m_Pipeline->GetVkPipelineLayout(), constant.ShaderStages, constant.Offset, value);
   }


   void VulkanGraphicsContext::DrawTriangles(const VertexBuffer& vertexBuffer, const uint32_t vertexCount, const uint32_t vertexOffset/*= 0*/) {
      BindDescriptorSets();
      Bind(vertexBuffer);
      GetVkCommandBuffer().draw(vertexCount, 1, vertexOffset, 0);
   }


   void VulkanGraphicsContext::DrawIndexed(const VertexBuffer& vertexBuffer, const IndexBuffer& indexBuffer, const uint32_t indexCount, const uint32_t vertexOffset/*= 0*/) {
      uint32_t count = indexCount ? indexCount : indexBuffer.GetCount();
      BindDescriptorSets();
      Bind(vertexBuffer);
      Bind(indexBuffer);
      GetVkCommandBuffer().drawIndexed(count, 1, 0, vertexOffset, 0);
   }


   vk::RenderPass VulkanGraphicsContext::GetVkRenderPass(BeginFrameOp operation) const {
      return m_RenderPasses.find(operation)->second;
   }


   vk::PipelineCache VulkanGraphicsContext::GetVkPipelineCache() const {
      return m_PipelineCache;
   }


   vk::SampleCountFlagBits VulkanGraphicsContext::GetNumSamples() const {
      return m_SampleCount;
   }


   vk::RenderPass VulkanGraphicsContext::CreateRenderPass(const std::vector<vk::AttachmentDescription2>& attachments) {

      std::vector<vk::AttachmentReference2> colorAttachmentRefs;
      std::vector<vk::AttachmentReference2> depthAttachmentRefs;
      std::vector<vk::AttachmentReference2> resolveColorAttachmentRefs;
      std::vector<vk::AttachmentReference2> resolveDepthAttachmentRefs;

      const bool isMultiSampled = m_SampleCount == vk::SampleCountFlagBits::e1 ? false : true;
      for (uint32_t i = 0; i < attachments.size(); ++i) {
         const vk::AttachmentDescription2& attachment = attachments[i];
         if (IsColorFormat(VkFormatToTextureFormat(attachment.format))) {
            if (isMultiSampled && (attachment.samples == vk::SampleCountFlagBits::e1)) {
               resolveColorAttachmentRefs.emplace_back(i, vk::ImageLayout::eColorAttachmentOptimal, vk::ImageAspectFlagBits::eColor);
            } else {
               colorAttachmentRefs.emplace_back(i, vk::ImageLayout::eColorAttachmentOptimal, vk::ImageAspectFlagBits::eColor);
            }
         } else {
            if (isMultiSampled && (attachment.samples == vk::SampleCountFlagBits::e1)) {
               resolveDepthAttachmentRefs.emplace_back(i, vk::ImageLayout::eDepthStencilAttachmentOptimal, vk::ImageAspectFlagBits::eDepth);
            } else {
               depthAttachmentRefs.emplace_back(i, vk::ImageLayout::eDepthStencilAttachmentOptimal, vk::ImageAspectFlagBits::eDepth);
            }
         }
      }

      vk::SubpassDescriptionDepthStencilResolve depthResolve = {
         {vk::ResolveModeFlagBits::eMin}    /*depthResolveMode*/,
         {vk::ResolveModeFlagBits::eNone}   /*stencilResolveMode*/,
         resolveDepthAttachmentRefs.data()  /*pDepthStencilResolveAttachment*/
      };

      vk::SubpassDescription2 subpass = {
         {}                                                  /*flags*/,
         vk::PipelineBindPoint::eGraphics                    /*pipelineBindPoint*/,
         0                                                   /*viewMask*/,
         0                                                   /*inputAttachmentCount*/,
         nullptr                                             /*pInputAttachments*/,
         static_cast<uint32_t>(colorAttachmentRefs.size())   /*colorAttachmentCount*/,
         colorAttachmentRefs.data()                          /*pColorAttachments*/,
         resolveColorAttachmentRefs.data()                   /*pResolveAttachments*/,
         depthAttachmentRefs.data()                          /*pDepthStencilAttachment*/,
         0                                                   /*preserveAttachmentCount*/,
         nullptr                                             /*pPreserveAttachments*/
      };
      if (resolveDepthAttachmentRefs.size() > 0) {
         subpass.pNext = &depthResolve;
      }

      // Use subpass dependencies for layout transitions
      std::array<vk::SubpassDependency2, 2> dependencies = {
         vk::SubpassDependency2 {
            VK_SUBPASS_EXTERNAL                                /*srcSubpass*/,
            0                                                  /*dstSubpass*/,
            vk::PipelineStageFlagBits::eFragmentShader         /*srcStageMask*/,
            vk::PipelineStageFlagBits::eEarlyFragmentTests     /*dstStageMask*/,
            vk::AccessFlagBits::eShaderRead                    /*srcAccessMask*/,
            vk::AccessFlagBits::eDepthStencilAttachmentWrite   /*dstAccessMask*/,
            vk::DependencyFlagBits::eByRegion                  /*dependencyFlags*/,
            0                                                  /*viewOffset*/
         }, 
         vk::SubpassDependency2 {
            0                                                  /*srcSubpass*/,
            VK_SUBPASS_EXTERNAL                                /*dstSubpass*/,
            vk::PipelineStageFlagBits::eLateFragmentTests      /*srcStageMask*/,
            vk::PipelineStageFlagBits::eFragmentShader         /*dstStageMask*/,
            vk::AccessFlagBits::eDepthStencilAttachmentWrite   /*srcAccessMask*/,
            vk::AccessFlagBits::eShaderRead                    /*dstAccessMask*/,
            vk::DependencyFlagBits::eByRegion                  /*dependencyFlags*/,
            0                                                  /*viewOffset*/
         }
      };

      return m_Device->GetVkDevice().createRenderPass2({
         {}                                         /*flags*/,
         static_cast<uint32_t>(attachments.size())  /*attachmentCount*/,
         attachments.data()                         /*pAttachments*/,
         1                                          /*subpassCount*/,
         &subpass                                   /*pSubpasses*/,
         static_cast<uint32_t>(dependencies.size()) /*dependencyCount*/,
         dependencies.data()                        /*pDependencies*/,
         0                                          /*correlatedViewMaskCount*/,
         nullptr                                    /*pCorrelatedviewMasks*/
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


   void VulkanGraphicsContext::BindDescriptorSets() {
      m_Pipeline->BindDescriptorSets(GetVkCommandBuffer(), GetFence());
   }


   void VulkanGraphicsContext::UnbindDescriptorSets() {
      m_Pipeline->UnbindDescriptorSets();
   }


   VulkanWindowGC::VulkanWindowGC(std::shared_ptr<VulkanDevice> device, const Window& window)
   : VulkanGraphicsContext {device}
   , m_Window {static_cast<GLFWwindow*>(window.GetNativeWindow())}
   , m_IsVSync(window.IsVSync())
   {
      m_SampleCount = static_cast<vk::SampleCountFlagBits>(window.GetMSAANumSamples());
      CreateSurface();
      CreateSwapChain();
      CreateImageViews();
      CreateColorImage();
      CreateDepthStencil();

      std::vector<vk::AttachmentDescription2> attachments;
      if (m_SampleCount == vk::SampleCountFlagBits::e1) {
         attachments = {
            {
               {}                                               /*flags*/,
               m_Format                                         /*format*/,
               m_SampleCount                                    /*samples*/,
               vk::AttachmentLoadOp::eClear                     /*loadOp*/,
               vk::AttachmentStoreOp::eStore                    /*storeOp*/,
               vk::AttachmentLoadOp::eDontCare                  /*stencilLoadOp*/,
               vk::AttachmentStoreOp::eDontCare                 /*stencilStoreOp*/,
               vk::ImageLayout::eUndefined                      /*initialLayout*/,
               vk::ImageLayout::ePresentSrcKHR                  /*finalLayout*/
            },
            {
               {}                                               /*flags*/,
               m_DepthFormat                                    /*format*/,
               m_SampleCount                                    /*samples*/,
               vk::AttachmentLoadOp::eClear                     /*loadOp*/,
               vk::AttachmentStoreOp::eDontCare                 /*storeOp*/,
               vk::AttachmentLoadOp::eDontCare                  /*stencilLoadOp*/,
               vk::AttachmentStoreOp::eDontCare                 /*stencilStoreOp*/,
               vk::ImageLayout::eUndefined                      /*initialLayout*/,
               vk::ImageLayout::eDepthStencilAttachmentOptimal  /*finalLayout*/
            }
         };
      } else {
         attachments = {
            {
               {}                                               /*flags*/,
               m_Format                                         /*format*/,
               m_SampleCount                                    /*samples*/,
               vk::AttachmentLoadOp::eClear                     /*loadOp*/,
               vk::AttachmentStoreOp::eStore                    /*storeOp*/,
               vk::AttachmentLoadOp::eDontCare                  /*stencilLoadOp*/,
               vk::AttachmentStoreOp::eDontCare                 /*stencilStoreOp*/,
               vk::ImageLayout::eUndefined                      /*initialLayout*/,
               vk::ImageLayout::eColorAttachmentOptimal         /*finalLayout*/
            },
            {
               {}                                               /*flags*/,
               m_DepthFormat                                    /*format*/,
               m_SampleCount                                    /*samples*/,
               vk::AttachmentLoadOp::eClear                     /*loadOp*/,
               vk::AttachmentStoreOp::eDontCare                 /*storeOp*/,
               vk::AttachmentLoadOp::eDontCare                  /*stencilLoadOp*/,
               vk::AttachmentStoreOp::eDontCare                 /*stencilStoreOp*/,
               vk::ImageLayout::eUndefined                      /*initialLayout*/,
               vk::ImageLayout::eDepthStencilAttachmentOptimal  /*finalLayout*/
            },
            {
               vk::AttachmentDescriptionFlags {}                /*flags*/,
               m_Format                                         /*format*/,
               vk::SampleCountFlagBits::e1                      /*samples*/,
               vk::AttachmentLoadOp::eDontCare                  /*loadOp*/,
               vk::AttachmentStoreOp::eStore                    /*storeOp*/,
               vk::AttachmentLoadOp::eDontCare                  /*stencilLoadOp*/,
               vk::AttachmentStoreOp::eDontCare                 /*stencilStoreOp*/,
               vk::ImageLayout::eUndefined                      /*initialLayout*/,
               vk::ImageLayout::ePresentSrcKHR                  /*finalLayout*/
            }
         };
      }
      // This is horrible.
      // Create four different render passes, just in case user decides they don't want to clear this or that attachment
      attachments[0].loadOp = vk::AttachmentLoadOp::eDontCare;
      attachments[1].loadOp = vk::AttachmentLoadOp::eDontCare;
      m_RenderPasses[BeginFrameOp::ClearNone] = CreateRenderPass(attachments);

      attachments[0].loadOp = vk::AttachmentLoadOp::eClear;
      attachments[1].loadOp = vk::AttachmentLoadOp::eDontCare;
      m_RenderPasses[BeginFrameOp::ClearColor] = CreateRenderPass(attachments);

      attachments[0].loadOp = vk::AttachmentLoadOp::eDontCare;
      attachments[1].loadOp = vk::AttachmentLoadOp::eClear;
      m_RenderPasses[BeginFrameOp::ClearDepth] = CreateRenderPass(attachments);

      attachments[0].loadOp = vk::AttachmentLoadOp::eClear;
      attachments[1].loadOp = vk::AttachmentLoadOp::eClear;
      m_RenderPasses[BeginFrameOp::ClearAll] = CreateRenderPass(attachments);

      CreateFramebuffers();

      CreateCommandPool();
      CreateCommandBuffers(static_cast<uint32_t>(m_SwapChainImages.size()));
      CreateSyncObjects();
      CreatePipelineCache();

      EventDispatcher::Connect<WindowResizeEvent, &VulkanWindowGC::OnWindowResize>(*this);
      EventDispatcher::Connect<WindowVSyncChangedEvent, &VulkanWindowGC::OnWindowVSyncChanged>(*this);

      // Set clear values for all framebuffer attachments with loadOp set to clear
      // The window graphics context has two attachments (color and depth) that are cleared at the start of the subpass and as such we need to set clear values for both
      glm::vec4 clearColor = window.GetClearColor();
      m_ClearValues = {
         vk::ClearColorValue {std::array<float, 4>{clearColor.r, clearColor.g, clearColor.b, clearColor.a}},
         vk::ClearDepthStencilValue {0.0f, 0}  // note: Pikzel uses reverse depth, so depth clear value is 0.0, rather than 1.0
      };

   }


   VulkanWindowGC::~VulkanWindowGC() {

      EventDispatcher::Disconnect<WindowVSyncChangedEvent, &VulkanWindowGC::OnWindowVSyncChanged>(*this);
      EventDispatcher::Disconnect<WindowResizeEvent, &VulkanWindowGC::OnWindowResize>(*this);

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
         for (auto& [beginFrameOp, renderPass] : m_RenderPasses) {
            DestroyRenderPass(renderPass);
         }
         m_RenderPasses.clear();
         DestroyDepthStencil();
         DestroyImageViews();
         DestroySwapChain(m_SwapChain);
         DestroySurface();
      }
   }


   void VulkanWindowGC::BeginFrame(const BeginFrameOp operation) {
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
      vk::Result result = m_Device->GetVkDevice().waitForFences(m_InFlightFences[m_CurrentFrame]->GetVkFence(), true, UINT64_MAX);

      vk::CommandBufferBeginInfo commandBufferBI = {
         vk::CommandBufferUsageFlagBits::eSimultaneousUse
      };
      m_CommandBuffers[m_CurrentImage].begin(commandBufferBI);

      // TODO: Not sure that this is the best place to begin render pass.
      //       What if you need/want multiple render passes?  How will the client control this?

      vk::RenderPassBeginInfo renderPassBI = {
         m_RenderPasses[operation]                    /*renderPass*/,
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

      m_Device->GetVkDevice().resetFences(m_InFlightFences[m_CurrentFrame]->GetVkFence());
      m_Device->GetGraphicsQueue().submit(si, m_InFlightFences[m_CurrentFrame]->GetVkFence());

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
      if (m_InitializedImGui) {
         PKZL_CORE_LOG_WARN("ImGui already initialised!");
         return;
      }
      m_DescriptorPoolImGui = CreateDescriptorPool(DescriptorBinding {0, 1, vk::DescriptorType::eCombinedImageSampler, {}}, 10);

      std::vector<vk::AttachmentDescription2> attachments;
      if (m_SampleCount == vk::SampleCountFlagBits::e1) {
         attachments = {
            {
               {}                                               /*flags*/,
               m_Format                                         /*format*/,
               m_SampleCount                                    /*samples*/,
               vk::AttachmentLoadOp::eDontCare                  /*loadOp*/,
               vk::AttachmentStoreOp::eStore                    /*storeOp*/,
               vk::AttachmentLoadOp::eDontCare                  /*stencilLoadOp*/,
               vk::AttachmentStoreOp::eDontCare                 /*stencilStoreOp*/,
               vk::ImageLayout::eUndefined                      /*initialLayout*/,
               vk::ImageLayout::ePresentSrcKHR                  /*finalLayout*/
            },
            {
               {}                                               /*flags*/,
               m_DepthFormat                                    /*format*/,
               m_SampleCount                                    /*samples*/,
               vk::AttachmentLoadOp::eDontCare                  /*loadOp*/,
               vk::AttachmentStoreOp::eDontCare                 /*storeOp*/,
               vk::AttachmentLoadOp::eDontCare                  /*stencilLoadOp*/,
               vk::AttachmentStoreOp::eDontCare                 /*stencilStoreOp*/,
               vk::ImageLayout::eUndefined                      /*initialLayout*/,
               vk::ImageLayout::eDepthStencilAttachmentOptimal  /*finalLayout*/
            }
         };
      } else {
         attachments = {
            {
               {}                                               /*flags*/,
               m_Format                                         /*format*/,
               m_SampleCount                                    /*samples*/,
               vk::AttachmentLoadOp::eDontCare                  /*loadOp*/,
               vk::AttachmentStoreOp::eStore                    /*storeOp*/,
               vk::AttachmentLoadOp::eDontCare                  /*stencilLoadOp*/,
               vk::AttachmentStoreOp::eDontCare                 /*stencilStoreOp*/,
               vk::ImageLayout::eUndefined                      /*initialLayout*/,
               vk::ImageLayout::eColorAttachmentOptimal         /*finalLayout*/
            },
            {
               {}                                               /*flags*/,
               m_DepthFormat                                    /*format*/,
               m_SampleCount                                    /*samples*/,
               vk::AttachmentLoadOp::eDontCare                  /*loadOp*/,
               vk::AttachmentStoreOp::eDontCare                 /*storeOp*/,
               vk::AttachmentLoadOp::eDontCare                  /*stencilLoadOp*/,
               vk::AttachmentStoreOp::eDontCare                 /*stencilStoreOp*/,
               vk::ImageLayout::eUndefined                      /*initialLayout*/,
               vk::ImageLayout::eDepthStencilAttachmentOptimal  /*finalLayout*/
            },
            {
               vk::AttachmentDescriptionFlags {}                /*flags*/,
               m_Format                                         /*format*/,
               vk::SampleCountFlagBits::e1                      /*samples*/,
               vk::AttachmentLoadOp::eDontCare                  /*loadOp*/,
               vk::AttachmentStoreOp::eStore                    /*storeOp*/,
               vk::AttachmentLoadOp::eDontCare                  /*stencilLoadOp*/,
               vk::AttachmentStoreOp::eDontCare                 /*stencilStoreOp*/,
               vk::ImageLayout::eUndefined                      /*initialLayout*/,
               vk::ImageLayout::ePresentSrcKHR                  /*finalLayout*/
            }
         };
      }
      m_RenderPassImGui = CreateRenderPass(attachments);

      ImGui::CreateContext();

      ImGuiIO& io = ImGui::GetIO();
      io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
      io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
      io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

      ImGui_ImplGlfw_InitForVulkan(m_Window, true);
      ImGui_ImplVulkan_InitInfo init_info = {
         .Instance        = m_Device->GetVkInstance(),
         .PhysicalDevice  = m_Device->GetVkPhysicalDevice(),
         .Device          = m_Device->GetVkDevice(),
         .QueueFamily     = m_Device->GetGraphicsQueueFamilyIndex(),
         .Queue           = m_Device->GetGraphicsQueue(),
         .PipelineCache   = m_PipelineCache,
         .DescriptorPool  = m_DescriptorPoolImGui,
         .Subpass         = 0,
         .MinImageCount   = static_cast<uint32_t>(m_SwapChainImages.size()),
         .ImageCount      = static_cast<uint32_t>(m_SwapChainImages.size()),
         .MSAASamples     = static_cast<VkSampleCountFlagBits>(GetNumSamples()),
         .Allocator       = nullptr, // TODO: proper allocator...
         .CheckVkResultFn = [](const VkResult err) {
            if (err != VK_SUCCESS) {
               throw std::runtime_error {"ImGui Vulkan error!"};
            }
         }
      };
      ImGui_ImplVulkan_Init(&init_info, m_RenderPassImGui);
      super::InitializeImGui();
      m_InitializedImGui = true;
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


   std::shared_ptr<VulkanFence> VulkanWindowGC::GetFence() {
      return m_InFlightFences[m_CurrentFrame];
   }


   uint32_t VulkanWindowGC::GetNumColorAttachments() const {
      return 1;
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
         if (availableFormat.format == vk::Format::eB8G8R8A8Srgb && availableFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
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

      uint32_t imageCount = swapChainSupport.Capabilities.minImageCount + 1;
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


   void VulkanWindowGC::CreateColorImage() {
      if (m_SampleCount != vk::SampleCountFlagBits::e1) {
         m_ColorImage = std::make_unique<VulkanImage>(m_Device, vk::ImageViewType::e2D, m_Extent.width, m_Extent.height, 1, 1, m_SampleCount, m_Format, vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eTransientAttachment | vk::ImageUsageFlagBits::eColorAttachment, vma::MemoryUsage::eGpuOnly);
         m_ColorImage->CreateImageViews(m_Format, vk::ImageAspectFlagBits::eColor);
      }
   }


   void VulkanWindowGC::DestroyColorImage() {
      m_ColorImage = nullptr;
   }


   void VulkanWindowGC::CreateDepthStencil() {
      m_DepthFormat = FindSupportedFormat(
         m_Device->GetVkPhysicalDevice(),
         {vk::Format::eD32SfloatS8Uint, vk::Format::eD32Sfloat, vk::Format::eD24UnormS8Uint, vk::Format::eD16UnormS8Uint, vk::Format::eD16Unorm},
         vk::ImageTiling::eOptimal,
         vk::FormatFeatureFlagBits::eDepthStencilAttachment
      );
      m_DepthImage = std::make_unique<VulkanImage>(m_Device, vk::ImageViewType::e2D, m_Extent.width, m_Extent.height, 1, 1, m_SampleCount, m_DepthFormat, vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eDepthStencilAttachment, vma::MemoryUsage::eGpuOnly);
      m_DepthImage->CreateImageViews(m_DepthFormat, vk::ImageAspectFlagBits::eDepth);
   }


   void VulkanWindowGC::DestroyDepthStencil() {
      m_DepthImage = nullptr;
   }


   void VulkanWindowGC::CreateImageViews() {
      for (auto& image : m_SwapChainImages) {
         image.CreateImageViews(m_Format, vk::ImageAspectFlagBits::eColor);
      }
   }


   void VulkanWindowGC::DestroyImageViews() {
      if (m_Device) {
         for (auto& image : m_SwapChainImages) {
            image.DestroyImageViews();
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
      std::vector<vk::ImageView> attachments;
      if (m_SampleCount == vk::SampleCountFlagBits::e1) {
         attachments = {
            nullptr,
            m_DepthImage->GetVkImageView()
         };
      } else {
         attachments = {
            m_ColorImage->GetVkImageView(),
            m_DepthImage->GetVkImageView(),
            nullptr
         };
      }
      vk::FramebufferCreateInfo ci = {
         {}                                        /*flags*/,
         m_RenderPasses[BeginFrameOp::ClearAll]    /*renderPass*/,
         static_cast<uint32_t>(attachments.size()) /*attachmentCount*/,
         attachments.data()                        /*pAttachments*/,
         m_Extent.width                            /*width*/,
         m_Extent.height                           /*height*/,
         1                                         /*layers*/
      };

      m_SwapChainFramebuffers.reserve(m_SwapChainImages.size());
      for (const auto& swapChainImage : m_SwapChainImages) {
         if (m_SampleCount == vk::SampleCountFlagBits::e1) {
            attachments[0] = swapChainImage.GetVkImageView();
         } else {
            attachments[2] = swapChainImage.GetVkImageView();
         }
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

      for (uint32_t i = 0; i < m_MaxFramesInFlight; ++i) {
         m_ImageAvailableSemaphores.emplace_back(m_Device->GetVkDevice().createSemaphore({}));
         m_RenderFinishedSemaphores.emplace_back(m_Device->GetVkDevice().createSemaphore({}));
         m_InFlightFences.emplace_back(std::make_shared<VulkanFence>(m_Device->GetVkDevice()));
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
         m_InFlightFences.clear();
      }
   }


   void VulkanWindowGC::RecreateSwapChain() {
      m_Device->GetVkDevice().waitIdle();

      SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(m_Device->GetVkPhysicalDevice(), m_Surface);
      vk::Extent2D extent = SelectSwapExtent(swapChainSupport.Capabilities);

      if ((extent.width > 0) && (extent.height > 0)) {
         DestroyImageViews();
         CreateSwapChain();
         CreateImageViews();

         DestroyColorImage();
         CreateColorImage();

         DestroyDepthStencil();
         CreateDepthStencil();

         DestroyFramebuffers();
         CreateFramebuffers();

         m_WantResize = false;
      }
   }


   void VulkanWindowGC::OnWindowResize(const WindowResizeEvent& event) {
      if (event.sender == m_Window && (event.width > 0) && (event.height > 0)) {
         m_WantResize = true;
      }
   }


   void VulkanWindowGC::OnWindowVSyncChanged(const WindowVSyncChangedEvent& event) {
      if (event.sender == m_Window) {
         m_IsVSync = event.isVSync;
         m_WantResize = true; // force swapchain recreate
      }
   }


   VulkanFramebufferGC::VulkanFramebufferGC(std::shared_ptr<VulkanDevice> device, VulkanFramebuffer* framebuffer)
   : VulkanGraphicsContext {device}
   , m_Framebuffer {framebuffer}
   {
      glm::vec4 clearColor = m_Framebuffer->GetClearColorValue();
      m_ClearValues.reserve(m_Framebuffer->GetVkAttachments().size());
      for (const auto& attachment : m_Framebuffer->GetVkAttachments()) {
         if (IsColorFormat(VkFormatToTextureFormat(attachment.format))) {
            m_ClearValues.emplace_back(vk::ClearColorValue {std::array<float, 4>{clearColor.r, clearColor.g, clearColor.b, clearColor.a}});
         } else {
            m_ClearValues.emplace_back(vk::ClearDepthStencilValue {static_cast<float>(m_Framebuffer->GetClearDepthValue()), 0});
         }
      };
      m_SampleCount = static_cast<vk::SampleCountFlagBits>(m_Framebuffer->GetMSAANumSamples());
      m_Extent = vk::Extent2D{m_Framebuffer->GetWidth(), m_Framebuffer->GetHeight()};

      for (auto& attachment : m_Framebuffer->GetVkAttachments()) {
         attachment.loadOp = vk::AttachmentLoadOp::eDontCare;
         attachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
      }
      m_RenderPasses[BeginFrameOp::ClearNone] = CreateRenderPass(m_Framebuffer->GetVkAttachments());

      for (auto& attachment : m_Framebuffer->GetVkAttachments()) {
         if ((m_Framebuffer->GetMSAANumSamples() == 1) || (attachment.samples != vk::SampleCountFlagBits::e1)) {
            if (IsColorFormat(VkFormatToTextureFormat(attachment.format))) {
               attachment.loadOp = vk::AttachmentLoadOp::eClear;
               attachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
            } else {
               attachment.loadOp = vk::AttachmentLoadOp::eDontCare;
               attachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
            }
         } else {
            attachment.loadOp = vk::AttachmentLoadOp::eDontCare;
            attachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
         }
      }
      m_RenderPasses[BeginFrameOp::ClearColor] = CreateRenderPass(m_Framebuffer->GetVkAttachments());

      for (auto& attachment : m_Framebuffer->GetVkAttachments()) {
         if ((m_Framebuffer->GetMSAANumSamples() == 1) || (attachment.samples != vk::SampleCountFlagBits::e1)) {
            if (IsColorFormat(VkFormatToTextureFormat(attachment.format))) {
               attachment.loadOp = vk::AttachmentLoadOp::eDontCare;
               attachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
            } else {
               attachment.loadOp = vk::AttachmentLoadOp::eClear;
               attachment.stencilLoadOp = vk::AttachmentLoadOp::eClear;
            }
         } else {
            attachment.loadOp = vk::AttachmentLoadOp::eDontCare;
            attachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
         }
      }
      m_RenderPasses[BeginFrameOp::ClearDepth] = CreateRenderPass(m_Framebuffer->GetVkAttachments());

      for (auto& attachment : m_Framebuffer->GetVkAttachments()) {
         if ((m_Framebuffer->GetMSAANumSamples() == 1) || (attachment.samples != vk::SampleCountFlagBits::e1)) {
            if (IsColorFormat(VkFormatToTextureFormat(attachment.format))) {
               attachment.loadOp = vk::AttachmentLoadOp::eClear;
               attachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
            } else {
               attachment.loadOp = vk::AttachmentLoadOp::eClear;
               attachment.stencilLoadOp = vk::AttachmentLoadOp::eClear;
            }
         } else {
            attachment.loadOp = vk::AttachmentLoadOp::eDontCare;
            attachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
         }
      }
      m_RenderPasses[BeginFrameOp::ClearAll] = CreateRenderPass(m_Framebuffer->GetVkAttachments());
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
         for (auto& [beginFrameOp, renderPass] : m_RenderPasses) {
            DestroyRenderPass(renderPass);
         }
         m_RenderPasses.clear();
      }
   }


   void VulkanFramebufferGC::BeginFrame(const BeginFrameOp operation) {
      PKZL_PROFILE_FUNCTION();

      vk::CommandBuffer cmd = GetVkCommandBuffer();
      cmd.begin({
         vk::CommandBufferUsageFlagBits::eSimultaneousUse
      });

      // TODO: Not sure that this is the best place to begin render pass.
      //       What if you need/want multiple render passes?  How will the client control this?

      vk::RenderPassBeginInfo renderPassBI = {
         m_RenderPasses[operation]                    /*renderPass*/,
         m_Framebuffer->GetVkFramebuffer()            /*framebuffer*/,
         { {0,0}, m_Extent }                          /*renderArea*/,
         static_cast<uint32_t>(m_ClearValues.size())  /*clearValueCount*/,
         m_ClearValues.data()                         /*pClearValues*/
      };
      cmd.beginRenderPass(renderPassBI, vk::SubpassContents::eInline);

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
      cmd.setViewport(0, viewport);

      vk::Rect2D scissor = {
         {0, 0},
         m_Extent
      };
      cmd.setScissor(0, scissor);
   }


   void VulkanFramebufferGC::EndFrame() {
      PKZL_PROFILE_FUNCTION();
      vk::CommandBuffer cmd = m_CommandBuffers.front();
      cmd.endRenderPass();  // TODO: think about where render passes should begin/end
      cmd.end();

      vk::SubmitInfo si = {
         0                /*waitSemaphoreCount*/,
         nullptr          /*pWaitSemaphores*/,
         nullptr          /*pWaitDstStageMask*/,
         1                /*commandBufferCount*/,
         &cmd             /*pCommandBuffers*/,
         0                /*signalSemaphoreCount*/,
         nullptr          /*pSignalSemaphores*/
      };

      m_Device->GetVkDevice().resetFences(m_InFlightFence->GetVkFence());
      m_Device->GetGraphicsQueue().submit(si, m_InFlightFence->GetVkFence());

      // Depth texture to shader read only here.
      // This is so that other "graphics contexts" can ask this framebuffer for the depth texture
      // and then sample from that.
      // This is not really how things should be done, as it can be much more efficient to do it
      // with render sub-passes instead.
      // TODO: subpasses
      if(m_Framebuffer->HasDepthAttachment()) {
         const auto& depthTexture = static_cast<const VulkanTexture&>(m_Framebuffer->GetDepthTexture());
         m_Device->PipelineBarrier(
            vk::PipelineStageFlagBits::eEarlyFragmentTests,
            vk::PipelineStageFlagBits::eFragmentShader,
            depthTexture.GetImage().Barrier(vk::ImageLayout::eDepthStencilAttachmentOptimal, vk::ImageLayout::eShaderReadOnlyOptimal, 0, 0, 0, 0)
         );
      }
   }


   void VulkanFramebufferGC::Bind(const Pipeline& pipeline) {
      const VulkanPipeline& vulkanPipeline = static_cast<const VulkanPipeline&>(pipeline);
      GetVkCommandBuffer().bindPipeline(vk::PipelineBindPoint::eGraphics, vulkanPipeline.GetVkPipelineFrontFaceCW());
      m_Pipeline = const_cast<VulkanPipeline*>(&vulkanPipeline);
      m_Pipeline->UnbindDescriptorSets();
   }


   void VulkanFramebufferGC::Unbind(const Pipeline&) {
      m_Pipeline = nullptr;
   }


   void VulkanFramebufferGC::SwapBuffers() {
      vk::Result result = m_Device->GetVkDevice().waitForFences(m_InFlightFence->GetVkFence(), true, UINT64_MAX);
   }


   vk::CommandBuffer VulkanFramebufferGC::GetVkCommandBuffer() {
      return m_CommandBuffers.front();
   }


   std::shared_ptr<VulkanFence> VulkanFramebufferGC::GetFence() {
      return m_InFlightFence;
   }


   uint32_t VulkanFramebufferGC::GetNumColorAttachments() const {
      return m_Framebuffer->GetNumColorAttachments();
   }


   void VulkanFramebufferGC::CreateSyncObjects() {
      m_InFlightFence = std::make_shared<VulkanFence>(m_Device->GetVkDevice());
   }


   void VulkanFramebufferGC::DestroySyncObjects() {
      if (m_Device && m_InFlightFence) {
         m_InFlightFence = nullptr;
      }
   }

}
