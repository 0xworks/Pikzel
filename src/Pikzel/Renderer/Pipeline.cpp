#include "Pikzel/Renderer/Pipeline.h"
#include "Pikzel/Renderer/GraphicsDevice.h"
#include "Pikzel/Renderer/Nri.h"
#include "Pikzel/Renderer/Shader.h"

#include <cassert>
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

}


Pipeline::Pipeline(GraphicsDevice& device, const GraphicsPipelineSettings& settings)
: m_Device(&device), m_VertexStride(settings.vertexStride) {
   assert(settings.vertexShader && "GraphicsPipelineSettings::vertexShader must be set");
   assert(settings.fragmentShader && "GraphicsPipelineSettings::fragmentShader must be set");
   if (settings.colorFormat == Format::Unknown) {
      throw std::invalid_argument("GraphicsPipelineSettings::colorFormat must be set");
   }
   // Checked on every API, so that a pipeline written and tested on Vulkan does not
   // fail only when it is first run on D3D12.
   for (const auto& attribute : settings.vertexAttributes) {
      if (attribute.semanticName.empty()) {
         throw std::invalid_argument("VertexAttribute::semanticName must be set");
      }
   }

   const auto& nri = device.GetNri();

   // An empty layout: the shaders read nothing but vertex data.
   nri::PipelineLayoutDesc layoutDesc = {};
   layoutDesc.shaderStages = nri::StageBits::VERTEX_SHADER | nri::StageBits::FRAGMENT_SHADER;
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
   desc.rasterization.frontCounterClockwise = false;
   desc.outputMerger.colors = &colorAttachment;
   desc.outputMerger.colorNum = 1;
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
: m_Device(std::exchange(other.m_Device, nullptr)), m_PipelineLayout(std::exchange(other.m_PipelineLayout, nullptr)), m_Pipeline(std::exchange(other.m_Pipeline, nullptr)), m_VertexStride(std::exchange(other.m_VertexStride, 0)) {}


Pipeline& Pipeline::operator=(Pipeline&& other) noexcept {
   if (this != &other) {
      Release();
      m_Device = std::exchange(other.m_Device, nullptr);
      m_PipelineLayout = std::exchange(other.m_PipelineLayout, nullptr);
      m_Pipeline = std::exchange(other.m_Pipeline, nullptr);
      m_VertexStride = std::exchange(other.m_VertexStride, 0);
   }
   return *this;
}


uint32_t Pipeline::GetVertexStride() const {
   return m_VertexStride;
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
