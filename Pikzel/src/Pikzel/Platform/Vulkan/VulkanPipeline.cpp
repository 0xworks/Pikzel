#include "VulkanPipeline.h"

#include "VulkanGraphicsContext.h"
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
         case ShaderType::Geometry: return vk::ShaderStageFlagBits::eGeometry;
         case ShaderType::Fragment: return vk::ShaderStageFlagBits::eFragment;
         case ShaderType::Compute: return vk::ShaderStageFlagBits::eCompute;
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


   VulkanPipeline::VulkanPipeline(std::shared_ptr<VulkanDevice> device, const PipelineSettings& settings)
   : m_Device {device} {
      CreateDescriptorSetLayouts(settings);
      CreatePipelineLayout();
      CreateComputePipeline(settings);
      CreateDescriptorPool();
   }


   VulkanPipeline::VulkanPipeline(std::shared_ptr<VulkanDevice> device, VulkanGraphicsContext& gc, const PipelineSettings& settings)
   : m_Device {device}
   {
      CreateDescriptorSetLayouts(settings);
      CreatePipelineLayout();
      CreateGraphicsPipeline(gc, settings);
      CreateDescriptorPool();
   }


   VulkanPipeline::~VulkanPipeline() {
      m_Device->GetVkDevice().waitIdle();
      DestroyDesciptorPool();
      DestroyPipeline();
      DestroyPipelineLayout();
      DestroyDescriptorSetLayouts();
   }


   vk::Pipeline VulkanPipeline::GetVkPipelineCompute() const {
      return m_PipelineCompute;
   }


   vk::Pipeline VulkanPipeline::GetVkPipelineFrontFaceCCW() const {
      return m_PipelineFrontFaceCCW;
   }


   vk::Pipeline VulkanPipeline::GetVkPipelineFrontFaceCW() const {
      return m_PipelineFrontFaceCW;
   }


   vk::PipelineLayout VulkanPipeline::GetVkPipelineLayout() const {
      return m_PipelineLayout;
   }


   const VulkanPushConstant& VulkanPipeline::GetPushConstant(const entt::id_type id) const {
      return m_PushConstants.at(id);
   }


   const VulkanResource& VulkanPipeline::GetResource(const entt::id_type id) const {
      return m_Resources.at(id);
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


   void VulkanPipeline::ReflectShaders() {
      PKZL_CORE_LOG_TRACE("Reflecting shaders");

      for (const auto& [shaderType, src] : m_ShaderSrcs) {
         spirv_cross::Compiler compiler(src);
         spirv_cross::ShaderResources resources = compiler.get_shader_resources();
         auto variables = compiler.get_active_interface_variables();

         for (const auto& pushConstantBuffer : resources.push_constant_buffers) {
            const auto& bufferType = compiler.get_type(pushConstantBuffer.base_type_id);
            uint32_t memberCount = static_cast<uint32_t>(bufferType.member_types.size());
            for (uint32_t i = 0; i < memberCount; ++i) {
               std::string pushConstantName = (pushConstantBuffer.name != "" ? (pushConstantBuffer.name + ".") : "") + compiler.get_member_name(bufferType.self, i);
               PKZL_CORE_LOG_TRACE("Found push constant range with name '{0}'", pushConstantName);
               auto pc = m_PushConstants.find(entt::hashed_string(pushConstantName.data()));
               if (pc == m_PushConstants.end()) {
                  const auto& type = compiler.get_type(bufferType.member_types[i]);
                  uint32_t offset = compiler.type_struct_member_offset(bufferType, i);
                  uint32_t size = static_cast<uint32_t>(compiler.get_declared_struct_member_size(bufferType, i));
                  m_PushConstants.emplace(entt::hashed_string(pushConstantName.data()), VulkanPushConstant {pushConstantName, SPIRTypeToDataType(type), ShaderTypeToVulkanShaderStage(shaderType), offset, size});
               } else {
                  pc->second.ShaderStages |= ShaderTypeToVulkanShaderStage(shaderType);
               }
            }
         }

         for (const auto& resource : resources.uniform_buffers) {
            const auto& name = resource.name;
            const auto& type = compiler.get_type(resource.type_id);
            std::vector<uint32_t> shape;
            if (type.array.size() > 0) {
               PKZL_CORE_LOG_ERROR(fmt::format("Uniform buffer object with name '{0}' is an array.  This is not currently supported by Pikzel!", name));
               shape.resize(type.array.size());  // number of dimensions of the array. 0 = its a scalar (i.e. not an array), 1 = 1D array, 2 = 2D array, etc...
               for (auto dim = 0; dim < shape.size(); ++dim) {
                  shape[dim] = type.array[dim];  // size of [dim]th dimension of the array
               }
            }

            uint32_t set = compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);
            uint32_t binding = compiler.get_decoration(resource.id, spv::DecorationBinding);
            PKZL_CORE_LOG_TRACE("Found uniform buffer at set {0}, binding {1} with name '{2}', R  rank is {3}", set, binding, name, shape.size());

            bool found = false;
            for (auto& [id, res] : m_Resources) {
               if ((res.DescriptorSet == set) && (res.Binding == binding)) {
                  if ((res.Name != name) || (res.Type != vk::DescriptorType::eUniformBuffer)) {
                     throw std::runtime_error {fmt::format("Descriptor set {0}, binding {1} is ambiguous.  Refers to different names (or types)!", set, binding)};
                  }
                  res.ShaderStages |= ShaderTypeToVulkanShaderStage(shaderType);
                  found = true;
                  break;
               }
            }
            entt::id_type id = entt::hashed_string(name.data());
            if (!found) {
               if (m_Resources.find(id) != m_Resources.end()) {
                  throw std::runtime_error {fmt::format("Shader resource name '{0}' is ambiguous.  Refers to different descriptor set bindings!", name)};
               } else {
                  m_Resources.emplace(id, VulkanResource {name, set, binding, vk::DescriptorType::eUniformBuffer, shape, ShaderTypeToVulkanShaderStage(shaderType)});
               }
            }
         }

         for (const auto& resource : resources.sampled_images) {
            const auto& name = resource.name;
            const auto& type = compiler.get_type(resource.type_id);
            std::vector<uint32_t> shape;
            if (type.array.size() > 0) {
               PKZL_CORE_LOG_ERROR(fmt::format("Sampler object with name '{0}' is an array.  This is not currently supported by Pikzel!", name));
               shape.resize(type.array.size());  // number of dimensions of the array. 0 = its a scalar (i.e. not an array), 1 = 1D array, 2 = 2D array, etc...
               for (auto dim = 0; dim < shape.size(); ++dim) {
                  shape[dim] = type.array[dim];  // size of [dim]th dimension of the array
               }
            }

            uint32_t set = compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);
            uint32_t binding = compiler.get_decoration(resource.id, spv::DecorationBinding);
            PKZL_CORE_LOG_TRACE("Found texture sampler at set {0}, binding {1} with name '{2}', rank is {3}", set, binding, name, shape.size());

            bool found = false;
            for (auto& [id, res] : m_Resources) {
               if ((res.DescriptorSet == set) && (res.Binding == binding)) {
                  if ((res.Name != name) || (res.Type != vk::DescriptorType::eCombinedImageSampler)) {
                     throw std::runtime_error {fmt::format("Descriptor set {0}, binding {1} is ambiguous.  Refers to different names (or types)!", set, binding)};
                  }
                  res.ShaderStages |= ShaderTypeToVulkanShaderStage(shaderType);
                  found = true;
                  break;
               }
            }
            entt::id_type id = entt::hashed_string(name.data());
            if (!found) {
               if (m_Resources.find(id) != m_Resources.end()) {
                  throw std::runtime_error {fmt::format("Shader resource name '{0}' is ambiguous.  Refers to different descriptor set bindings!", name)};
               } else {
                  m_Resources.emplace(id, VulkanResource {name, set, binding, vk::DescriptorType::eCombinedImageSampler, shape, ShaderTypeToVulkanShaderStage(shaderType)});
               }
            }
         }

         for (const auto& resource : resources.storage_images) {
            const auto& name = resource.name;
            const auto& type = compiler.get_type(resource.type_id);
            std::vector<uint32_t> shape;
            if (type.array.size() > 0) {
               PKZL_CORE_LOG_ERROR(fmt::format("Stogate image object with name '{0}' is an array.  This is not currently supported by Pikzel!", name));
               shape.resize(type.array.size());  // number of dimensions of the array. 0 = its a scalar (i.e. not an array), 1 = 1D array, 2 = 2D array, etc...
               for (auto dim = 0; dim < shape.size(); ++dim) {
                  shape[dim] = type.array[dim];  // size of [dim]th dimension of the array
               }
            }
            uint32_t set = compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);
            uint32_t binding = compiler.get_decoration(resource.id, spv::DecorationBinding);
            PKZL_CORE_LOG_TRACE("Found storage image at set {0}, binding {1} with name '{2}', rank is {3}", set, binding, name, shape.size());

            bool found = false;
            for (auto& [id, res] : m_Resources) {
               if ((res.DescriptorSet == set) && (res.Binding == binding)) {
                  if ((res.Name != name) || (res.Type != vk::DescriptorType::eStorageImage)) {
                     throw std::runtime_error {fmt::format("Descriptor set {0}, binding {1} is ambiguous.  Refers to different names (or types)!", set, binding)};
                  }
                  res.ShaderStages |= ShaderTypeToVulkanShaderStage(shaderType);
                  found = true;
                  break;
               }
            }
            entt::id_type id = entt::hashed_string(name.data());
            if (!found) {
               if (m_Resources.find(id) != m_Resources.end()) {
                  throw std::runtime_error {fmt::format("Shader resource name '{0}' is ambiguous.  Refers to different descriptor set bindings!", name)};
               } else {
                  m_Resources.emplace(id, VulkanResource {name, set, binding, vk::DescriptorType::eStorageImage, shape, ShaderTypeToVulkanShaderStage(shaderType)});
               }
            }
         }
      }
   }


   void VulkanPipeline::CreateDescriptorSetLayouts(const PipelineSettings& settings) {
      for (const auto& [shaderType, path] : settings.Shaders) {
         m_ShaderSrcs.emplace_back(shaderType, ReadFile<uint32_t>(path));
      }

      ReflectShaders();

      std::vector<std::vector<vk::DescriptorSetLayoutBinding>> layoutBindings;
      for (const auto& [id, resource] : m_Resources) {
         if (layoutBindings.size() <= resource.DescriptorSet) {
            layoutBindings.resize(resource.DescriptorSet + 1);
         }
         layoutBindings[resource.DescriptorSet].emplace_back(resource.Binding, resource.Type, resource.GetCount(), resource.ShaderStages);
      }

      for (const auto& layoutBinding : layoutBindings) {
         vk::DescriptorSetLayoutCreateInfo ci = {
            {}                                            /*flags*/,
            static_cast<uint32_t>(layoutBinding.size())   /*bindingCount*/,
            layoutBinding.data()                          /*pBindings*/
         };

         m_DescriptorSetLayouts.emplace_back(m_Device->GetVkDevice().createDescriptorSetLayout(ci));
         m_DescriptorSetInstances.emplace_back();
         m_DescriptorSetFences.emplace_back();
         m_DescriptorSetIndices.emplace_back(0);
         m_DescriptorSetPending.emplace_back(false);
         m_DescriptorSetBound.emplace_back(false);
      }
   }


   std::shared_ptr<VulkanDevice> VulkanPipeline::GetDevice() {
      return m_Device;
   }


   const std::vector<vk::DescriptorSetLayout>& VulkanPipeline::GetVkDescriptorSetLayouts() const {
      return m_DescriptorSetLayouts;
   }


   void VulkanPipeline::DestroyDescriptorSetLayouts() {
      if (m_Device) {
         for (const auto descriptorSetLayout : m_DescriptorSetLayouts) {
            m_Device->GetVkDevice().destroy(descriptorSetLayout);
         }
         m_DescriptorSetLayouts.clear();
      }
   }


   void VulkanPipeline::CreatePipelineLayout() {
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
         {}                                                    /*flags*/,
         static_cast<uint32_t>(m_DescriptorSetLayouts.size())  /*setLayoutCount*/,
         m_DescriptorSetLayouts.data()                         /*pSetLayouts*/,
         static_cast<uint32_t>(vkPushConstantRanges.size())    /*pushConstantRangeCount*/,
         vkPushConstantRanges.data()                           /*pPushConstantRanges*/
      });
   }


   void VulkanPipeline::DestroyPipelineLayout() {
      ;
      if (m_Device && m_PipelineLayout) {
         m_Device->GetVkDevice().destroy(m_PipelineLayout);
         m_PipelineLayout = nullptr;
      }
   }


   void VulkanPipeline::CreateComputePipeline(const PipelineSettings& settings) {
      m_PipelineBindPoint = vk::PipelineBindPoint::eCompute;

      vk::ComputePipelineCreateInfo pipelineCI;
      pipelineCI.layout = m_PipelineLayout;

      const auto& [shaderType, src] = m_ShaderSrcs.front();
      pipelineCI.stage = {
         vk::PipelineShaderStageCreateFlags {}           /*flags*/,
         ShaderTypeToVulkanShaderStage(shaderType)       /*stage*/,
         CreateShaderModule(shaderType, src)             /*module*/,
         "main"                                          /*name*/,
         nullptr                                         /*pSpecializationInfo*/
      };

      // .value works around issue in Vulkan.hpp (refer https://github.com/KhronosGroup/Vulkan-Hpp/issues/659)
      m_PipelineCompute = m_Device->GetVkDevice().createComputePipeline({}, pipelineCI).value;

      // Shader modules are no longer needed once the pipeline has been created
      DestroyShaderModule(pipelineCI.stage.module);
      m_ShaderSrcs.clear();
   }


   void VulkanPipeline::CreateGraphicsPipeline(const VulkanGraphicsContext& gc, const PipelineSettings& settings) {
      m_PipelineBindPoint = vk::PipelineBindPoint::eGraphics;

      vk::GraphicsPipelineCreateInfo pipelineCI;
      pipelineCI.layout = m_PipelineLayout;
      pipelineCI.renderPass = gc.GetVkRenderPass(BeginFrameOp::ClearAll);

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
         {vk::CullModeFlagBits::eBack}    /*cullMode*/,
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
      // Note: Doesn't actually matter what you set here, it gets overwritten by the dynamic states
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
         // vk::DynamicState::eFrontFaceEXT,    // at time of writing, VK_EXT_extended_dynamic_state is not available in the nvidia general release drivers, so for now we will create two separate pipelines!
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
         {}                           /*flags*/,
         true                         /*depthTestEnable*/,
         true                         /*depthWriteEnable*/,
         vk::CompareOp::eLessOrEqual  /*depthCompareOp*/,          // LE for skybox
         false                        /*depthBoundsTestEnable*/,
         false                        /*stencilTestEnable*/,
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
      vk::PipelineMultisampleStateCreateInfo multisampleState = {
         {}                                                              /*flags*/,
         gc.GetNumSamples()                                              /*rasterizationSamples*/,
         m_Device->GetEnabledPhysicalDeviceFeatures().sampleRateShading  /*sampleShadingEnable*/,
         1.0f                                                            /*minSampleShading*/,
         nullptr                                                         /*pSampleMask*/,
         false                                                           /*alphaToCoverageEnable*/,
         false                                                           /*alphaToOneEnable*/
      };
      pipelineCI.pMultisampleState = &multisampleState;

      // Vertex input descriptions 
      // Specifies the vertex input parameters for a pipeline
      auto bindingDescription = vk::VertexInputBindingDescription {
         0,
         settings.BufferLayout.GetStride(),
         vk::VertexInputRate::eVertex
      };

      auto attributeDescriptions = GetAttributeDescriptions(settings.BufferLayout);

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

      // .value works around issue in Vulkan.hpp (refer https://github.com/KhronosGroup/Vulkan-Hpp/issues/659)
      m_PipelineFrontFaceCCW = m_Device->GetVkDevice().createGraphicsPipeline(gc.GetVkPipelineCache(), pipelineCI).value;

      rasterizationState.frontFace = vk::FrontFace::eClockwise;
      m_PipelineFrontFaceCW = m_Device->GetVkDevice().createGraphicsPipeline(gc.GetVkPipelineCache(), pipelineCI).value;

      // Shader modules are no longer needed once the graphics pipeline has been created
      for (auto& shaderStage : shaderStages) {
         DestroyShaderModule(shaderStage.module);
      }
      m_ShaderSrcs.clear();
   }


   void VulkanPipeline::DestroyPipeline() {
      if (m_Device) {
         if (m_PipelineFrontFaceCCW) {
            m_Device->GetVkDevice().destroy(m_PipelineFrontFaceCCW);
            m_PipelineFrontFaceCCW = nullptr;
         }
         if (m_PipelineFrontFaceCW) {
            m_Device->GetVkDevice().destroy(m_PipelineFrontFaceCW);
            m_PipelineFrontFaceCW = nullptr;
         }
         if (m_PipelineCompute) {
            m_Device->GetVkDevice().destroy(m_PipelineCompute);
            m_PipelineCompute = nullptr;
         }
      }
   }


   void VulkanPipeline::CreateDescriptorPool() {
      constexpr uint32_t howMany = 10; // We don't really know at this point how many descriptor sets might end up being needed.
                                       // We just create "some" here, to allow toy examples to work (TODO: may need to provide a way to set this amount)
                                       // For more complicated rendering tasks, Pikzel provides various "Renderer" classes which manage their own descriptor
                                       // sets.

      std::unordered_map<vk::DescriptorType, uint32_t> descriptorTypeCount;
      for (const auto& [id, resource] : m_Resources) {
         ++descriptorTypeCount[resource.Type];
      }

      std::vector<vk::DescriptorPoolSize> poolSizes;
      for (const auto [type, count] : descriptorTypeCount) {
         poolSizes.emplace_back(type, howMany * count);
      }
      
      if (!poolSizes.empty()) {
         vk::DescriptorPoolCreateInfo descriptorPoolCI = {
            {}                                                               /*flags*/,
            howMany * static_cast<uint32_t>(m_DescriptorSetLayouts.size())   /*maxSets*/,
            static_cast<uint32_t>(poolSizes.size())                          /*poolSizeCount*/,
            poolSizes.data()                                                 /*pPoolSizes*/
         };
         m_DescriptorPool = m_Device->GetVkDevice().createDescriptorPool(descriptorPoolCI);
      }
   }


   void VulkanPipeline::DestroyDesciptorPool() {
      if (m_Device && m_DescriptorPool) {
         m_Device->GetVkDevice().destroy(m_DescriptorPool);
      }
      m_DescriptorPool = nullptr;
      m_DescriptorSetInstances.clear();
      m_DescriptorSetFences.clear();
      m_DescriptorSetIndices.clear();
      m_DescriptorSetPending.clear();
      m_DescriptorSetBound.clear();
   }


   vk::DescriptorSet VulkanPipeline::AllocateDescriptorSet(const uint32_t set) {
      vk::DescriptorSetAllocateInfo allocInfo = {
         m_DescriptorPool,
         1,
         &m_DescriptorSetLayouts[set]
      };

      m_DescriptorSetInstances[set].emplace_back(m_Device->GetVkDevice().allocateDescriptorSets(allocInfo).front());
      m_DescriptorSetBound[set].emplace_back(false);
      m_DescriptorSetFences[set].emplace_back(nullptr);
      m_DescriptorSetIndices[set] = m_DescriptorSetInstances[set].size() - 1;
      m_DescriptorSetPending[set] = true;
      return m_DescriptorSetInstances[set].back();
   }


   vk::DescriptorSet VulkanPipeline::GetVkDescriptorSet(const uint32_t set) {
      // if we have not created an instance of the specified descriptor set yet, allocate a new one and return.
      // otherwise, look for an instance that isn't currently in use (has not already been bound to the pipeline, and is not still in use by some previously submitted render commands) and return that one
      // if cannot find one that isn't in use, allocate a new one and return
      if (m_DescriptorSetInstances[set].empty()) {
         return AllocateDescriptorSet(set);
      }
      uint32_t i = m_DescriptorSetIndices[set];
      do {
         if (!m_DescriptorSetBound[set][i] && (!m_DescriptorSetFences[set][i] || (m_Device->GetVkDevice().getFenceStatus(m_DescriptorSetFences[set][i]->GetVkFence()) == vk::Result::eSuccess))) {
            m_DescriptorSetIndices[set] = i;
            m_DescriptorSetPending[set] = true;
            m_DescriptorSetFences[set][i] = nullptr;
            return m_DescriptorSetInstances[set][i];
         }
         if (++i == m_DescriptorSetInstances[set].size()) {
            i = 0;
         }
      } while(i != m_DescriptorSetIndices[set]);
      return AllocateDescriptorSet(set);
   }


   void VulkanPipeline::BindDescriptorSets(vk::CommandBuffer commandBuffer, std::shared_ptr<VulkanFence> fence) {
      for (uint32_t i = 0; i < m_DescriptorSetInstances.size(); ++i) {
         if (m_DescriptorSetPending[i]) {
            commandBuffer.bindDescriptorSets(m_PipelineBindPoint, GetVkPipelineLayout(), i, m_DescriptorSetInstances[i][m_DescriptorSetIndices[i]], nullptr);
            m_DescriptorSetFences[i][m_DescriptorSetIndices[i]] = fence;
            m_DescriptorSetPending[i] = false;
            m_DescriptorSetBound[i][m_DescriptorSetIndices[i]] = true;
         }
      }
   }


   void VulkanPipeline::UnbindDescriptorSets() {
      for (auto& bound : m_DescriptorSetBound) {
         std::fill(bound.begin(), bound.end(), false);
      }
   }

}
