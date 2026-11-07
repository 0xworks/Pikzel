#pragma once

#include "Pikzel/Renderer/Format.h"

#include <cstdint>
#include <string>
#include <vector>

namespace nri {
struct Pipeline;
struct PipelineLayout;
}

namespace Pikzel {

class GraphicsDevice;
class Shader;

enum class PrimitiveTopology : uint8_t {
   PointList,
   LineList,
   LineStrip,
   TriangleList,
   TriangleStrip,
};

// Which triangles to discard, by facing. Triangles whose vertices appear counter-clockwise
// on screen are front-facing. This is the convention glTF uses (with right-handed, +Y up
// coordinates, which the engine also follows), so imported meshes need no rewinding.
enum class CullMode : uint8_t {
   None,
   Front,
   Back,
};

// How the fragment shader's output is combined with what is already in the render target.
enum class BlendMode : uint8_t {
   // Output replaces the existing color.
   Opaque,
   // Standard "over" compositing: result = source * source.a + destination * (1 - source.a).
   // On an sRGB render target this blending happens on linear values, before encoding.
   Alpha,
};

// How a fragment's depth is compared with the depth buffer. The fragment passes (is drawn)
// when "fragment depth <op> stored depth" holds.
enum class CompareOp : uint8_t {
   Never,
   Less,
   Equal,
   LessEqual,
   Greater,
   NotEqual,
   GreaterEqual,
   Always,
};

// The kinds of resource a shader can reach through a descriptor set. Each corresponds to a
// Slang (HLSL) resource type and register kind:
//
//   ConstantBuffer   ConstantBuffer<T>   b registers
//   Texture          Texture2D           t registers
//   Sampler          SamplerState        s registers
enum class DescriptorType : uint8_t {
   ConstantBuffer,
   Texture,
   Sampler,
};

// A run of descriptors of one type, bound to consecutive registers starting at
// baseRegister. For example {DescriptorType::Texture, 0, 2} covers t0 and t1.
struct DescriptorRange {
   DescriptorType type = DescriptorType::ConstantBuffer;
   uint32_t baseRegister = 0;
   uint32_t count = 1;
};

// The resources a shader expects in one register space, for example `register(t0, space2)`.
// A DescriptorSet with this layout provides them. In Vulkan, the register space becomes the
// descriptor set number (Slang maps spaceN to set N).
struct DescriptorSetLayout {
   uint32_t registerSpace = 0;
   std::vector<DescriptorRange> ranges;
};

// One input to the vertex shader, read from each vertex in the vertex buffer.
//
// The graphics APIs match attributes to shader inputs in different ways, so an attribute
// names its shader input in both. Vulkan matches by location number: [[vk::location(n)]]
// on the shader input. D3D12 matches by HLSL semantic: for example `: COLOR` (name "COLOR",
// index 0) or `: TEXCOORD1` (name "TEXCOORD", index 1). Both must be filled in for a
// pipeline to work on either API.
struct VertexAttribute {
   uint32_t location = 0;
   std::string semanticName;
   uint32_t semanticIndex = 0;
   Format format = Format::Unknown;
   // Byte offset of the attribute within a vertex.
   uint32_t offset = 0;
};

struct GraphicsPipelineSettings {
   // Compiled entry points for each stage, loaded for the same device as the pipeline.
   const Shader* vertexShader = nullptr;
   const Shader* fragmentShader = nullptr;

   // Layout of one vertex in the vertex buffer: its attributes, and the distance in bytes
   // from one vertex to the next. There is a single vertex buffer for now.
   std::vector<VertexAttribute> vertexAttributes;
   uint32_t vertexStride = 0;

   // The pipeline layout: which resources the shaders read, besides vertex data.
   //
   // descriptorSets lists the descriptor sets, in the order CommandList::BindDescriptorSet
   // refers to them by index. A common arrangement groups resources by how often they
   // change, for example one set for per-frame data and one per material, so that each set
   // is bound only when its contents change.
   //
   // pushConstantsSize is the size in bytes of the push constants block: a small amount of
   // data (128 bytes is the most every GPU supports) recorded directly into the command
   // buffer with CommandList::PushConstants, ideal for data that changes with every draw.
   // In shaders the block is `[[vk::push_constant]] ConstantBuffer<T> name : register(b0, space0);`.
   // Space 0 is therefore reserved for push constants when they are used.
   std::vector<DescriptorSetLayout> descriptorSets;
   uint32_t pushConstantsSize = 0;

   PrimitiveTopology topology = PrimitiveTopology::TriangleList;
   CullMode cullMode = CullMode::Back;
   BlendMode blendMode = BlendMode::Opaque;

   // Format of the render target the pipeline draws into, e.g. SwapChain::GetFormat().
   Format colorFormat = Format::Unknown;

   // Format of the depth buffer, e.g. SwapChain::GetDepthFormat(). Unknown means the
   // pipeline is used without a depth buffer, and does no depth testing.
   //
   // The defaults suit the engine's reversed-Z depth convention, in which near is 1 and far
   // is 0 (see Camera.h): a fragment is drawn if it is nearer, i.e. greater, than what the
   // depth buffer holds.
   Format depthFormat = Format::Unknown;
   CompareOp depthCompareOp = CompareOp::Greater;
   bool isDepthWriteEnabled = true;
};

// A compiled graphics pipeline: shaders plus all the fixed-function state (vertex layout,
// rasterisation, blending, attachment formats) needed to draw with them.
//
// Creating a pipeline is expensive, because the driver compiles the shaders to GPU machine
// code at this point. Pipelines are created up front and reused, never per frame.
//
// Each pipeline owns its pipeline layout: the description of the resources (descriptor
// sets, push constants) its shaders expect, built from GraphicsPipelineSettings. NRI allows
// one layout to be shared by several pipelines, which would let descriptor sets stay bound
// across a pipeline change. The engine does not need that yet, so the layout stays an
// internal part of the pipeline. Descriptor sets are still compatible between pipelines
// whose set layouts are identical.
//
// The settings are only read during construction; nothing in them needs to outlive it.
class Pipeline {
public:
   Pipeline(GraphicsDevice& device, const GraphicsPipelineSettings& settings);
   ~Pipeline();

   Pipeline(const Pipeline&) = delete;
   Pipeline& operator=(const Pipeline&) = delete;
   Pipeline(Pipeline&& other) noexcept;
   Pipeline& operator=(Pipeline&& other) noexcept;

   uint32_t GetVertexStride() const;
   uint32_t GetPushConstantsSize() const;
   const std::vector<DescriptorSetLayout>& GetDescriptorSetLayouts() const;

   // Renderer internal.
   nri::Pipeline& GetNriPipeline() const;
   nri::PipelineLayout& GetNriPipelineLayout() const;

private:
   void Release();

   GraphicsDevice* m_Device = nullptr;
   nri::PipelineLayout* m_PipelineLayout = nullptr;
   nri::Pipeline* m_Pipeline = nullptr;
   std::vector<DescriptorSetLayout> m_DescriptorSetLayouts;
   uint32_t m_VertexStride = 0;
   uint32_t m_PushConstantsSize = 0;
};

}
