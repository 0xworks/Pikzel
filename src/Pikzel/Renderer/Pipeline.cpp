#include "Pikzel/Renderer/Pipeline.h"
#include "Pikzel/Renderer/GraphicsDevice.h"
#include "Pikzel/Renderer/Nri.h"
#include "Pikzel/Renderer/Shader.h"

#include <cassert>
#include <cstddef>
#include <format>
#include <stdexcept>
#include <utility>

namespace Pikzel {

namespace {

nri::Topology ToNri(const PrimitiveTopology topology) {
   switch (topology) {
      case PrimitiveTopology::PointList:
         return nri::Topology::POINT_LIST;
      case PrimitiveTopology::LineList:
         return nri::Topology::LINE_LIST;
      case PrimitiveTopology::LineStrip:
         return nri::Topology::LINE_STRIP;
      case PrimitiveTopology::TriangleStrip:
         return nri::Topology::TRIANGLE_STRIP;
      case PrimitiveTopology::TriangleList:
      default:
         return nri::Topology::TRIANGLE_LIST;
   }
}


nri::CullMode ToNri(const CullMode cullMode) {
   switch (cullMode) {
      case CullMode::Front:
         return nri::CullMode::FRONT;
      case CullMode::Back:
         return nri::CullMode::BACK;
      case CullMode::None:
      default:
         return nri::CullMode::NONE;
   }
}


nri::CompareOp ToNri(const CompareOp compareOp) {
   switch (compareOp) {
      case CompareOp::Never:
         return nri::CompareOp::NEVER;
      case CompareOp::Less:
         return nri::CompareOp::LESS;
      case CompareOp::Equal:
         return nri::CompareOp::EQUAL;
      case CompareOp::LessEqual:
         return nri::CompareOp::LESS_EQUAL;
      case CompareOp::Greater:
         return nri::CompareOp::GREATER;
      case CompareOp::NotEqual:
         return nri::CompareOp::NOT_EQUAL;
      case CompareOp::GreaterEqual:
         return nri::CompareOp::GREATER_EQUAL;
      case CompareOp::Always:
      default:
         return nri::CompareOp::ALWAYS;
   }
}


nri::DescriptorType ToNri(const DescriptorType type) {
   switch (type) {
      case DescriptorType::Texture:
         return nri::DescriptorType::TEXTURE;
      case DescriptorType::Sampler:
         return nri::DescriptorType::SAMPLER;
      case DescriptorType::ConstantBuffer:
      default:
         return nri::DescriptorType::CONSTANT_BUFFER;
   }
}


void ValidateSettings(const GraphicsDevice& device, const GraphicsPipelineSettings& settings) {
   assert(settings.vertexShader && "GraphicsPipelineSettings::vertexShader must be set");
   assert(settings.fragmentShader && "GraphicsPipelineSettings::fragmentShader must be set");
   if (settings.colorFormat == Format::Unknown) {
      throw std::invalid_argument("GraphicsPipelineSettings::colorFormat must be set");
   }
   if (settings.depthFormat != Format::Unknown && !IsDepthFormat(settings.depthFormat)) {
      throw std::invalid_argument("GraphicsPipelineSettings::depthFormat must be a depth format");
   }

   // Checked on every API, so that a pipeline written and tested on Vulkan does not
   // fail only when it is first run on D3D12.
   for (const auto& attribute : settings.vertexAttributes) {
      if (attribute.semanticName.empty()) {
         throw std::invalid_argument("VertexAttribute::semanticName must be set");
      }
   }

   // Push constants live in register space 0 (see GraphicsPipelineSettings), so no
   // descriptor set may use it. Each set also needs a space of its own.
   for (std::size_t i = 0; i < settings.descriptorSets.size(); ++i) {
      const auto space = settings.descriptorSets[i].registerSpace;
      if (settings.pushConstantsSize > 0 && space == 0) {
         throw std::invalid_argument("Register space 0 is reserved for push constants; descriptor sets must use other spaces");
      }
      for (std::size_t j = 0; j < i; ++j) {
         if (settings.descriptorSets[j].registerSpace == space) {
            throw std::invalid_argument(std::format("Two descriptor sets use register space {}", space));
         }
      }
   }

   // NRI requires push constants to be a whole number of 32-bit values.
   if (settings.pushConstantsSize % 4 != 0) {
      throw std::invalid_argument("GraphicsPipelineSettings::pushConstantsSize must be a multiple of 4");
   }
   const auto maxPushConstantsSize = device.GetNri().GetDeviceDesc(device.GetDevice()).pipelineLayout.rootConstantMaxSize;
   if (settings.pushConstantsSize > maxPushConstantsSize) {
      throw std::invalid_argument(std::format("GraphicsPipelineSettings::pushConstantsSize is {} bytes, but this device supports at most {}", settings.pushConstantsSize, maxPushConstantsSize));
   }
}

}


Pipeline::Pipeline(GraphicsDevice& device, const GraphicsPipelineSettings& settings)
: m_Device(&device), m_DescriptorSetLayouts(settings.descriptorSets), m_VertexStride(settings.vertexStride), m_PushConstantsSize(settings.pushConstantsSize) {
   ValidateSettings(device, settings);

   const auto& nri = device.GetNri();
   constexpr auto shaderStages = nri::StageBits::VERTEX_SHADER | nri::StageBits::FRAGMENT_SHADER;

   // Translate the descriptor set layouts. The ranges of all sets go into one array, which
   // the per-set descriptions point into, so it is filled completely before any pointers
   // into it are taken.
   std::vector<nri::DescriptorRangeDesc> ranges;
   for (const auto& set : settings.descriptorSets) {
      for (const auto& range : set.ranges) {
         ranges.push_back({
            .baseRegisterIndex = range.baseRegister,
            .descriptorNum = range.count,
            .descriptorType = ToNri(range.type),
            .shaderStages = shaderStages,
            .flags = range.count > 1 ? nri::DescriptorRangeBits::ARRAY : nri::DescriptorRangeBits::NONE,
         });
      }
   }

   std::vector<nri::DescriptorSetDesc> sets;
   std::size_t firstRange = 0;
   for (const auto& set : settings.descriptorSets) {
      sets.push_back({
         .registerSpace = set.registerSpace,
         .ranges = ranges.data() + firstRange,
         .rangeNum = static_cast<uint32_t>(set.ranges.size()),
      });
      firstRange += set.ranges.size();
   }

   // Push constants are "root constants" in NRI (and D3D12) terms: register b0 in the root
   // register space, 0.
   const nri::RootConstantDesc pushConstants = {
      .registerIndex = 0,
      .size = settings.pushConstantsSize,
      .shaderStages = shaderStages,
   };

   nri::PipelineLayoutDesc layoutDesc = {};
   layoutDesc.rootRegisterSpace = 0;
   layoutDesc.rootConstants = settings.pushConstantsSize > 0 ? &pushConstants : nullptr;
   layoutDesc.rootConstantNum = settings.pushConstantsSize > 0 ? 1 : 0;
   layoutDesc.descriptorSets = sets.data();
   layoutDesc.descriptorSetNum = static_cast<uint32_t>(sets.size());
   layoutDesc.shaderStages = shaderStages;
   CheckNri(nri.CreatePipelineLayout(device.GetDevice(), layoutDesc, m_PipelineLayout), "CreatePipelineLayout");

   // The entry point name is only used by Vulkan, where a SPIR-V module may hold several
   // entry points. A DXIL shader has exactly one, so D3D12 has no use for it.
   const auto vertexBytecode = settings.vertexShader->GetBytecode();
   const auto fragmentBytecode = settings.fragmentShader->GetBytecode();
   const nri::ShaderDesc shaders[] = {
      {.stage = nri::StageBits::VERTEX_SHADER, .bytecode = vertexBytecode.data(), .size = vertexBytecode.size(), .entryPointName = settings.vertexShader->GetEntryPoint().c_str()},
      {.stage = nri::StageBits::FRAGMENT_SHADER, .bytecode = fragmentBytecode.data(), .size = fragmentBytecode.size(), .entryPointName = settings.fragmentShader->GetEntryPoint().c_str()},
   };

   // One vertex buffer ("stream") in binding slot 0.
   const nri::VertexStreamDesc vertexStream = {
      .bindingSlot = 0,
      .stride = static_cast<uint16_t>(settings.vertexStride),
   };

   // NRI takes both ways of naming the shader input and uses the one for its backend. The
   // semantic name pointers stay valid because settings outlives pipeline creation.
   std::vector<nri::VertexAttributeDesc> attributes;
   attributes.reserve(settings.vertexAttributes.size());
   for (const auto& attribute : settings.vertexAttributes) {
      attributes.push_back({
         .d3d = {attribute.semanticName.c_str(), attribute.semanticIndex},
         .vk = {attribute.location},
         .offset = attribute.offset,
         .format = Pikzel::ToNri(attribute.format),
         .streamIndex = 0,
      });
   }

   const nri::VertexInputDesc vertexInput = {
      .attributes = attributes.data(),
      .attributeNum = static_cast<uint8_t>(attributes.size()),
      .streams = &vertexStream,
      .streamNum = 1,
   };

   nri::ColorAttachmentDesc colorAttachment = {
      .format = Pikzel::ToNri(settings.colorFormat),
      .colorWriteMask = nri::ColorWriteBits::RGBA,
   };
   if (settings.blendMode == BlendMode::Alpha) {
      // Alpha itself accumulates coverage: the result is as opaque as the source and
      // destination together.
      colorAttachment.blendEnabled = true;
      colorAttachment.colorBlend = {nri::BlendFactor::SRC_ALPHA, nri::BlendFactor::ONE_MINUS_SRC_ALPHA, nri::BlendOp::ADD};
      colorAttachment.alphaBlend = {nri::BlendFactor::ONE, nri::BlendFactor::ONE_MINUS_SRC_ALPHA, nri::BlendOp::ADD};
   }

   nri::GraphicsPipelineDesc desc = {};
   desc.pipelineLayout = m_PipelineLayout;
   desc.vertexInput = attributes.empty() ? nullptr : &vertexInput;
   desc.inputAssembly.topology = ToNri(settings.topology);
   desc.rasterization.fillMode = nri::FillMode::SOLID;
   desc.rasterization.cullMode = ToNri(settings.cullMode);
   desc.rasterization.frontCounterClockwise = true;
   desc.outputMerger.colors = &colorAttachment;
   desc.outputMerger.colorNum = 1;
   if (settings.depthFormat != Format::Unknown) {
      desc.outputMerger.depthStencilFormat = Pikzel::ToNri(settings.depthFormat);
      desc.outputMerger.depth.compareOp = ToNri(settings.depthCompareOp);
      desc.outputMerger.depth.write = settings.isDepthWriteEnabled;
   }
   desc.shaders = shaders;
   desc.shaderNum = static_cast<uint32_t>(std::size(shaders));

   const auto result = nri.CreateGraphicsPipeline(device.GetDevice(), desc, m_Pipeline);
   if (result != nri::Result::SUCCESS) {
      Release();
      CheckNri(result, "CreateGraphicsPipeline");
   }
}


Pipeline::~Pipeline() {
   Release();
}


Pipeline::Pipeline(Pipeline&& other) noexcept
: m_Device(std::exchange(other.m_Device, nullptr)), m_PipelineLayout(std::exchange(other.m_PipelineLayout, nullptr)), m_Pipeline(std::exchange(other.m_Pipeline, nullptr)), m_DescriptorSetLayouts(std::exchange(other.m_DescriptorSetLayouts, {})), m_VertexStride(std::exchange(other.m_VertexStride, 0)), m_PushConstantsSize(std::exchange(other.m_PushConstantsSize, 0)) {}


Pipeline& Pipeline::operator=(Pipeline&& other) noexcept {
   if (this != &other) {
      Release();
      m_Device = std::exchange(other.m_Device, nullptr);
      m_PipelineLayout = std::exchange(other.m_PipelineLayout, nullptr);
      m_Pipeline = std::exchange(other.m_Pipeline, nullptr);
      m_DescriptorSetLayouts = std::exchange(other.m_DescriptorSetLayouts, {});
      m_VertexStride = std::exchange(other.m_VertexStride, 0);
      m_PushConstantsSize = std::exchange(other.m_PushConstantsSize, 0);
   }
   return *this;
}


uint32_t Pipeline::GetVertexStride() const {
   return m_VertexStride;
}


uint32_t Pipeline::GetPushConstantsSize() const {
   return m_PushConstantsSize;
}


const std::vector<DescriptorSetLayout>& Pipeline::GetDescriptorSetLayouts() const {
   return m_DescriptorSetLayouts;
}


nri::Pipeline& Pipeline::GetNriPipeline() const {
   return *m_Pipeline;
}


nri::PipelineLayout& Pipeline::GetNriPipelineLayout() const {
   return *m_PipelineLayout;
}


// The caller is responsible for making sure the GPU is no longer using the pipeline.
void Pipeline::Release() {
   if (m_Device) {
      const auto& nri = m_Device->GetNri();
      nri.DestroyPipeline(m_Pipeline);
      nri.DestroyPipelineLayout(m_PipelineLayout);
      m_Pipeline = nullptr;
      m_PipelineLayout = nullptr;
      m_Device = nullptr;
   }
}

}
