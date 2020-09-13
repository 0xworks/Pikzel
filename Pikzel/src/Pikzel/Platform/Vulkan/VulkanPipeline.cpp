#include "vkpch.h"
#include "VulkanPipeline.h"

#include "VulkanWindowGC.h"
#include "Pikzel/Core/Utility.h"
#include "Pikzel/Core/Window.h"
#include "Pikzel/Renderer/ShaderUtil.h"

#include <spirv_cross/spirv_cross.hpp>

#include <map>

namespace Pikzel {

   static vk::Format DataTypeToVkFormat(DataType type) {
      switch (type) {
         case DataType::Bool:     return vk::Format::eR8Sint;
         case DataType::Int:      return vk::Format::eR32Sint;
         case DataType::UInt:     return vk::Format::eR32Uint;
         case DataType::Float:    return vk::Format::eR32Sfloat;
         case DataType::Double:   return vk::Format::eR64Sfloat;
         case DataType::BVec2:    return vk::Format::eR8G8Sint;
         case DataType::BVec3:    return vk::Format::eR8G8B8Sint;
         case DataType::BVec4:    return vk::Format::eR8G8B8A8Sint;
         case DataType::IVec2:    return vk::Format::eR32G32Sint;
         case DataType::IVec3:    return vk::Format::eR32G32B32Sint;
         case DataType::IVec4:    return vk::Format::eR32G32B32A32Sint;
         case DataType::UVec2:    return vk::Format::eR32G32Uint;
         case DataType::UVec3:    return vk::Format::eR32G32B32Uint;
         case DataType::UVec4:    return vk::Format::eR32G32B32A32Uint;
         case DataType::Vec2:     return vk::Format::eR32G32Sfloat;
         case DataType::Vec3:     return vk::Format::eR32G32B32Sfloat;
         case DataType::Vec4:     return vk::Format::eR32G32B32A32Sfloat;
         case DataType::DVec2:    return vk::Format::eR64G64Sfloat;
         case DataType::DVec3:    return vk::Format::eR64G64B64Sfloat;
         case DataType::DVec4:    return vk::Format::eR64G64B64A64Sfloat;
      }
      PKZL_CORE_ASSERT(false, "Unknown DataType for VkFormat!");
      return {};
   }


   static vk::ShaderStageFlagBits ShaderTypeToVulkanShaderStage(ShaderType type) {
      switch (type) {
         case ShaderType::Vertex: return vk::ShaderStageFlagBits::eVertex;
         case ShaderType::Fragment: return vk::ShaderStageFlagBits::eFragment;
      }

      PKZL_CORE_ASSERT(false, "Unknown ShaderType!");
      return {};
   }


   static auto GetAttributeDescriptions(const BufferLayout& layout) {
      std::vector<vk::VertexInputAttributeDescription> attributeDescriptions;
      uint32_t location = 0;
      for (const auto& element : layout) {
         attributeDescriptions.emplace_back(
            location++                            /*location*/,
            0                                     /*binding*/,
            DataTypeToVkFormat(element.Type)      /*format*/,
            static_cast<uint32_t>(element.Offset) /*offset*/
         );
      }
      return attributeDescriptions;
   }


   VulkanPipeline::VulkanPipeline(std::shared_ptr<VulkanDevice> device, VulkanGraphicsContext& gc, const PipelineSettings& settings)
   : m_Device(device)
   {
      CreateDescriptorSetLayout(settings);
      CreatePipelineLayout(settings);
      CreatePipeline(gc, settings);
   }


   VulkanPipeline::~VulkanPipeline() {
      m_Device->GetVkDevice().waitIdle();
      DestroyPipeline();
      DestroyPipelineLayout();
      DestroyDescriptorSetLayout();
   }


   vk::Pipeline VulkanPipeline::GetVkPipeline() const {
      return m_Pipeline;
   }


   vk::PipelineLayout VulkanPipeline::GetVkPipelineLayout() const {
      return m_PipelineLayout;
   }


   const ShaderPushConstant& VulkanPipeline::GetPushConstant(const std::string& name) const {
      return m_PushConstants.at(name);
   }


   vk::ShaderModule VulkanPipeline::CreateShaderModule(ShaderType type, const std::vector<uint32_t>& src) {
      PKZL_CORE_ASSERT(m_Device, "Attempted to use null device!");
      vk::ShaderModuleCreateInfo ci = {
         {},
         src.size() * sizeof(uint32_t),
         src.data()
      };
      return m_Device->GetVkDevice().createShaderModule(ci);
   }


   void VulkanPipeline::DestroyShaderModule(vk::ShaderModule& module) {
      if (m_Device && module) {
         m_Device->GetVkDevice().destroy(module);
         module = nullptr;
      }
   }


   void VulkanPipeline::CreateDescriptorSetLayout(const PipelineSettings& settings) {
      // Basically connects the different shader stages to descriptors for binding uniform buffers, image samplers, etc.
      // So every shader binding should map to one descriptor set layout binding

      for (const auto& [shaderType, path] : settings.Shaders) {
         m_ShaderSrcs.emplace_back(shaderType, ReadFile<uint32_t>(path));
      }

      // Reflect shaders here...

      // TODO
      //vk::DescriptorSetLayoutBinding uboLayoutBinding = {
      //   0                                   /*binding*/,
      //   vk::DescriptorType::eUniformBuffer  /*descriptorType*/,
      //   1                                   /*descriptorCount*/,
      //   {vk::ShaderStageFlagBits::eVertex}  /*stageFlags*/,
      //   nullptr                             /*pImmutableSamplers*/
      //};

      std::vector<vk::DescriptorSetLayoutBinding> layoutBindings = { /* TODO */ };

      m_DescriptorSetLayout = m_Device->GetVkDevice().createDescriptorSetLayout({
         {}                                           /*flags*/,
         static_cast<uint32_t>(layoutBindings.size()) /*bindingCount*/,
         layoutBindings.data()                        /*pBindings*/
      });

   }


   void VulkanPipeline::DestroyDescriptorSetLayout() {
      if (m_Device && m_DescriptorSetLayout) {
         m_Device->GetVkDevice().destroy(m_DescriptorSetLayout);
         m_DescriptorSetLayout = nullptr;
      }
   }


   void VulkanPipeline::CreatePipelineLayout(const PipelineSettings& settings) {
      // Create the pipeline layout that is used to generate the rendering pipelines that are based on this descriptor set layout
      // In a more complex scenario you would have different pipeline layouts for different descriptor set layouts that could be reused

      // parse push constants
      for (const auto& [shaderType, src] : m_ShaderSrcs) {
         spirv_cross::Compiler compiler(src);
         spirv_cross::ShaderResources resources = compiler.get_shader_resources();
         for (const auto& pushConstantBuffer : resources.push_constant_buffers) {
            const auto& bufferType = compiler.get_type(pushConstantBuffer.base_type_id);
            uint32_t memberCount = static_cast<uint32_t>(bufferType.member_types.size());
            for (uint32_t i = 0; i < memberCount; ++i) {
               std::string pushConstantName = (pushConstantBuffer.name != "" ? (pushConstantBuffer.name + ".") : "") + compiler.get_member_name(bufferType.self, i);
               PKZL_CORE_LOG_TRACE("Found push constant range with name '{0}'", pushConstantName);
               auto pc = m_PushConstants.find(pushConstantName);
               if (pc == m_PushConstants.end()) {
                  const auto& type = compiler.get_type(bufferType.member_types[i]);
                  uint32_t offset = compiler.type_struct_member_offset(bufferType, i);
                  uint32_t size = static_cast<uint32_t>(compiler.get_declared_struct_member_size(bufferType, i));
                  m_PushConstants.emplace(pushConstantName, ShaderPushConstant {pushConstantName, SPIRTypeToDataType(type), ShaderTypeToVulkanShaderStage(shaderType), offset, size});
               } else {
                  pc->second.ShaderStages |= ShaderTypeToVulkanShaderStage(shaderType);
               }
            }
         }
      }

      // Vulkan spec requires that we do not declare more than one push constant range per shader stage,
      // so figure out "unique" ranges here...
      struct PushConstantRange {
         uint32_t MinOffset = ~0;
         uint32_t MaxOffset = 0;
      };
      std::map<vk::ShaderStageFlags, PushConstantRange> pushConstantRanges;
      for (const auto& [name, pushConstant] : m_PushConstants) {
         PushConstantRange& range = pushConstantRanges[pushConstant.ShaderStages];
         range.MinOffset = std::min(range.MinOffset, pushConstant.Offset);
         range.MaxOffset = std::max(range.MaxOffset, pushConstant.Offset + pushConstant.Size);
      }

      std::vector<vk::PushConstantRange> vkPushConstantRanges;
      for (const auto& [shaderStages, range] : pushConstantRanges) {
         vkPushConstantRanges.emplace_back(shaderStages, range.MinOffset, range.MaxOffset - range.MinOffset);
      }

      m_PipelineLayout = m_Device->GetVkDevice().createPipelineLayout({
         {}                                                  /*flags*/,
         1                                                   /*setLayoutCount*/,
         &m_DescriptorSetLayout                              /*pSetLayouts*/,
         static_cast<uint32_t>(vkPushConstantRanges.size())  /*pushConstantRangeCount*/,
         vkPushConstantRanges.data()                         /*pPushConstantRanges*/
      });
   }


   void VulkanPipeline::DestroyPipelineLayout() {
      ;
      if (m_Device && m_PipelineLayout) {
         m_Device->GetVkDevice().destroy(m_PipelineLayout);
         m_PipelineLayout = nullptr;
      }
   }


   void VulkanPipeline::CreatePipeline(const VulkanGraphicsContext& gc, const PipelineSettings& settings) {
      vk::GraphicsPipelineCreateInfo pipelineCI;
      pipelineCI.layout = m_PipelineLayout;
      pipelineCI.renderPass = gc.GetVkRenderPass();

      // Input assembly state describes how primitives are assembled
      // This pipeline will assemble vertex data as a triangle lists
      vk::PipelineInputAssemblyStateCreateInfo inputAssemblyState = {
         {}                                   /*flags*/,
         vk::PrimitiveTopology::eTriangleList /*topology*/,
         false                                /*primitiveRestartEnable*/
      };
      pipelineCI.pInputAssemblyState = &inputAssemblyState;

      // Rasterization state
      vk::PipelineRasterizationStateCreateInfo rasterizationState = {
         {}                               /*flags*/,
         false                            /*depthClampEnable*/,
         false                            /*rasterizerDiscardEnable*/,
         vk::PolygonMode::eFill           /*polygonMode*/,
         {vk::CullModeFlagBits::eNone}    /*cullMode*/,
         vk::FrontFace::eCounterClockwise /*frontFace*/,
         false                            /*depthBiasEnable*/,
         0.0f                             /*depthBiasConstantFactor*/,
         0.0f                             /*depthBiasClamp*/,
         0.0f                             /*depthBiasSlopeFactor*/,
         1.0f                             /*lineWidth*/
      };
      pipelineCI.pRasterizationState = &rasterizationState;

      // Color blend state describes how blend factors are calculated (if used)
      // We need one blend attachment state per color attachment (even if blending is not used)
      vk::PipelineColorBlendAttachmentState colorBlendAttachmentState = {
         true                                     /*blendEnable*/,
         vk::BlendFactor::eSrcAlpha               /*srcColorBlendFactor*/,
         vk::BlendFactor::eOneMinusSrcAlpha       /*dstColorBlendFactor*/,
         vk::BlendOp::eAdd                        /*colorBlendOp*/,
         vk::BlendFactor::eOne                    /*srcAlphaBlendFactor*/,
         vk::BlendFactor::eZero                   /*dstAlphaBlendFactor*/,
         vk::BlendOp::eAdd                        /*alphaBlendOp*/,
         {vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA } /*colorWriteMask*/
      };

      vk::PipelineColorBlendStateCreateInfo colorBlendState = {
         {}                         /*flags*/,
         false                      /*logicOpEnable*/,
         vk::LogicOp::eCopy         /*logicOp*/,
         1                          /*attachmentCount*/,
         &colorBlendAttachmentState /*pAttachments*/,
         {{0.0f}}                   /*blendConstants*/
      };
      pipelineCI.pColorBlendState = &colorBlendState;

      // Viewport state sets the number of viewports and scissor used in this pipeline
      // Note: Doesn't actually matter what you set here, it gets overridden by the dynamic states (see below)
      vk::Viewport viewport = {
         0.0f, 720.f,
         1280.0f, -720.0f,
         0.0f, 1.0f
      };

      vk::Rect2D scissor = {
         {0, 0},
         {1280, 720}
      };

      vk::PipelineViewportStateCreateInfo viewportState = {
         {}          /*flags*/,
         1           /*viewportCount*/,
         &viewport   /*pViewports*/,
         1           /*scissorCount*/,
         &scissor    /*pScissors*/
      };
      pipelineCI.pViewportState = &viewportState;

      // Enable dynamic states
      // Most states are baked into the pipeline, but there are still a few dynamic states that can be changed within a command buffer
      // To be able to change these we need do specify which dynamic states will be changed using this pipeline.
      // Their actual states are set later on in the command buffer.
      // We will set the viewport and scissor using dynamic states
      std::array<vk::DynamicState, 2> dynamicStates = {
         vk::DynamicState::eViewport,
         vk::DynamicState::eScissor
      };

      vk::PipelineDynamicStateCreateInfo dynamicState = {
         {}                                           /*flags*/,
         static_cast<uint32_t>(dynamicStates.size())  /*dynamicStateCount*/,
         dynamicStates.data()                         /*pDynamicStates*/
      };
      pipelineCI.pDynamicState = &dynamicState;

      // Depth and stencil state containing depth and stencil compare and test operations
      // We only use depth tests and want depth tests and writes to be enabled and compare with less
      vk::PipelineDepthStencilStateCreateInfo depthStencilState = {
         {}                    /*flags*/,
         true                  /*depthTestEnable*/,
         true                  /*depthWriteEnable*/,
         vk::CompareOp::eLess  /*depthCompareOp*/,
         false                 /*depthBoundsTestEnable*/,
         false                 /*stencilTestEnable*/,
         {
            vk::StencilOp::eKeep    /*failOp*/,
            vk::StencilOp::eKeep    /*passOp*/,
            vk::StencilOp::eKeep    /*depthFailOp*/,
            vk::CompareOp::eNever   /*compareOp*/,
            0                       /*compareMask*/,
            0                       /*writeMask*/,
            0                       /*reference*/
         }                     /*front*/,
         {
            vk::StencilOp::eKeep    /*failOp*/,
            vk::StencilOp::eKeep    /*passOp*/,
            vk::StencilOp::eKeep    /*depthFailOp*/,
            vk::CompareOp::eNever   /*compareOp*/,
            0                       /*compareMask*/,
            0                       /*writeMask*/,
            0                       /*reference*/
         }                     /*back*/,
         0.0f                  /*minDepthBounds*/,
         1.0f                  /*maxDepthBounds*/
      };
      pipelineCI.pDepthStencilState = &depthStencilState;

      // Multi sampling state
      // This example does not make use of multi sampling (for anti-aliasing), the state must still be set and passed to the pipeline
      vk::PipelineMultisampleStateCreateInfo multisampleState = {
         {}                          /*flags*/,
         vk::SampleCountFlagBits::e1 /*rasterizationSamples*/,
         false                       /*sampleShadingEnable*/,
         1.0f                        /*minSampleShading*/,
         nullptr                     /*pSampleMask*/,
         false                       /*alphaToCoverageEnable*/,
         false                       /*alphaToOneEnable*/
      };
      pipelineCI.pMultisampleState = &multisampleState;

      // Vertex input descriptions 
      // Specifies the vertex input parameters for a pipeline
      auto bindingDescription = vk::VertexInputBindingDescription {
         0,
         settings.VertexBuffer.GetLayout().GetStride(),
         vk::VertexInputRate::eVertex
      };

      auto attributeDescriptions = GetAttributeDescriptions(settings.VertexBuffer.GetLayout());

      // Vertex input state used for pipeline creation
      vk::PipelineVertexInputStateCreateInfo vertexInputState = {
         {}                                                    /*flags*/,
         1                                                     /*vertexBindingDescriptionCount*/,
         &bindingDescription                                   /*pVertexBindingDescriptions*/,
         static_cast<uint32_t>(attributeDescriptions.size())   /*vertexAttributeDescriptionCount*/,
         attributeDescriptions.data()                          /*pVertexAttributeDescriptions*/
      };
      pipelineCI.pVertexInputState = &vertexInputState;

      std::vector<vk::PipelineShaderStageCreateInfo> shaderStages;
      shaderStages.reserve(settings.Shaders.size());
      for (const auto& [shaderType, src] : m_ShaderSrcs) {
         shaderStages.emplace_back(
            vk::PipelineShaderStageCreateFlags {}           /*flags*/,
            ShaderTypeToVulkanShaderStage(shaderType)       /*stage*/,
            CreateShaderModule(shaderType, src)             /*module*/,
            "main"                                          /*name*/,
            nullptr                                         /*pSpecializationInfo*/
         );
      }
      pipelineCI.stageCount = static_cast<uint32_t>(shaderStages.size());
      pipelineCI.pStages = shaderStages.data();

      // .value works around bug in Vulkan.hpp (refer https://github.com/KhronosGroup/Vulkan-Hpp/issues/659)
      m_Pipeline = m_Device->GetVkDevice().createGraphicsPipeline(gc.GetVkPipelineCache(), pipelineCI).value;

      // Shader modules are no longer needed once the graphics pipeline has been created
      for (auto& shaderStage : shaderStages) {
         DestroyShaderModule(shaderStage.module);
      }
      m_ShaderSrcs.clear();
   }


   void VulkanPipeline::DestroyPipeline() {
      ;
      if (m_Device && m_Pipeline) {
         m_Device->GetVkDevice().destroy(m_Pipeline);
      }
   }

}
