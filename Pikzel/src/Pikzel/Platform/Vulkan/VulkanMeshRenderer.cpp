#include "VulkanMeshRenderer.h"

#include "VulkanPipeline.h"
#include "VulkanTexture.h"

namespace Pikzel {

   std::unique_ptr<MeshRenderer> MeshRenderer::Create(GraphicsContext& gc, const Model& model) {
      return std::make_unique<VulkanMeshRenderer>(gc, model);
   }


   VulkanMeshRenderer::VulkanMeshRenderer(GraphicsContext& gc, const Model& model)
   : MeshRenderer {gc, model}
   , m_Device {static_cast<VulkanPipeline&>(*m_Pipeline).GetDevice()}
   {
      CreateDescriptorPool(model);
      CreateDescriptorSets(model);
   }


   VulkanMeshRenderer::~VulkanMeshRenderer() {
      m_Device->GetVkDevice().waitIdle();
      DestroyDescriptorSets();
      DestroyDesciptorPool();
   }


   void VulkanMeshRenderer::CreateDescriptorPool(const Model& model) {
      vk::DescriptorPoolSize poolSize = {vk::DescriptorType::eCombinedImageSampler, static_cast<uint32_t>(model.Meshes.size() * 2)};
      vk::DescriptorPoolCreateInfo descriptorPoolCI = {
         vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet | vk::DescriptorPoolCreateFlagBits::eUpdateAfterBind  /*flags*/,
         static_cast<uint32_t>(model.Meshes.size())                                                                 /*maxSets*/,
         1                                                                                                          /*poolSizeCount*/,
         &poolSize                                                                                                  /*pPoolSizes*/
      };
      m_DescriptorPool = m_Device->GetVkDevice().createDescriptorPool(descriptorPoolCI);
   }


   void VulkanMeshRenderer::DestroyDesciptorPool() {
      if (m_Device && m_DescriptorPool) {
         m_Device->GetVkDevice().destroy(m_DescriptorPool);
      }
   }


   void VulkanMeshRenderer::CreateDescriptorSets(const Model& model) {

      VulkanPipeline& vkp = static_cast<VulkanPipeline&>(*m_Pipeline);

      const auto& descriptorSetLayouts = vkp.GetVkDescriptorSetLayouts();
      if (descriptorSetLayouts.size() != 2) {
         throw std::runtime_error("VulkanMeshRenderer expects exactly 2 descriptor set: one for the lights, and one for the material textures - check which shaders you constructed the pipeline with.");
      }

      vk::DescriptorSetAllocateInfo allocInfo = {
         m_DescriptorPool,
         1,
         &descriptorSetLayouts[1]
      };

      m_DescriptorSets.reserve(model.Meshes.size());
      for (const auto& mesh : model.Meshes) {
         m_DescriptorSets.push_back(m_Device->GetVkDevice().allocateDescriptorSets(allocInfo).front());

         const auto& diffuseTexture = static_cast<const VulkanTexture2D&>(*mesh.DiffuseTexture);
         vk::DescriptorImageInfo diffuseTextureImageDescriptor = {
            diffuseTexture.GetVkSampler()             /*sampler*/,
            diffuseTexture.GetVkImageView()           /*imageView*/,
            vk::ImageLayout::eShaderReadOnlyOptimal   /*imageLayout*/
         };

         const auto& specularTexture = static_cast<const VulkanTexture2D&>(*mesh.SpecularTexture);
         vk::DescriptorImageInfo specularTextureImageDescriptor = {
            specularTexture.GetVkSampler()             /*sampler*/,
            specularTexture.GetVkImageView()           /*imageView*/,
            vk::ImageLayout::eShaderReadOnlyOptimal    /*imageLayout*/
         };

         std::array<vk::WriteDescriptorSet, 2> writeDescriptorSets = {
            vk::WriteDescriptorSet {
               m_DescriptorSets.back()                                                     /*dstSet*/,
               0                                                                           /*dstBinding*/,
               0                                                                           /*dstArrayElement*/,
               1                                                                           /*descriptorCount*/,
               vk::DescriptorType::eCombinedImageSampler                                   /*descriptorType*/,
               &diffuseTextureImageDescriptor                                              /*pImageInfo*/,
               nullptr                                                                     /*pBufferInfo*/,
               nullptr                                                                     /*pTexelBufferView*/
            },
            vk::WriteDescriptorSet {
               m_DescriptorSets.back()                                                     /*dstSet*/,
               1                                                                           /*dstBinding*/,
               0                                                                           /*dstArrayElement*/,
               1                                                                           /*descriptorCount*/,
               vk::DescriptorType::eCombinedImageSampler                                   /*descriptorType*/,
               &specularTextureImageDescriptor                                             /*pImageInfo*/,
               nullptr                                                                     /*pBufferInfo*/,
               nullptr                                                                     /*pTexelBufferView*/
            }
         };

         m_Device->GetVkDevice().updateDescriptorSets(writeDescriptorSets, nullptr);
      }
   }


   void VulkanMeshRenderer::DestroyDescriptorSets() {
      if (m_Device) {
         m_Device->GetVkDevice().freeDescriptorSets(m_DescriptorPool, m_DescriptorSets);
      }
      m_DescriptorSets.clear();
   }


   void VulkanMeshRenderer::Draw(GraphicsContext& gc, const Mesh& mesh) const {
      const VulkanPipeline& vulkanPipeline = static_cast<const VulkanPipeline&>(*m_Pipeline);
      VulkanGraphicsContext& vgc = static_cast<VulkanGraphicsContext&>(gc);
      vgc.GetVkCommandBuffer().bindDescriptorSets(vk::PipelineBindPoint::eGraphics, vulkanPipeline.GetVkPipelineLayout(), 1, m_DescriptorSets[mesh.Index], nullptr);
      gc.DrawIndexed(*mesh.VertexBuffer, *mesh.IndexBuffer);
   }

}
