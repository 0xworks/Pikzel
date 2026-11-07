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

// Which triangles to discard, by facing. Triangles whose vertices appear clockwise on
// screen are front-facing.
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

   PrimitiveTopology topology = PrimitiveTopology::TriangleList;
   CullMode cullMode = CullMode::Back;
   BlendMode blendMode = BlendMode::Opaque;

   // Format of the render target the pipeline draws into, e.g. SwapChain::GetFormat().
   Format colorFormat = Format::Unknown;
};

// A compiled graphics pipeline: shaders plus all the fixed-function state (vertex layout,
// rasterisation, blending, attachment formats) needed to draw with them.
//
// Creating a pipeline is expensive, because the driver compiles the shaders to GPU machine
// code at this point. Pipelines are created up front and reused, never per frame.
//
// Each pipeline owns its pipeline layout: the description of the resources (descriptor
// sets, constants) its shaders expect. NRI allows layouts to be shared between pipelines,
// but until the engine binds resources there is nothing to share, so the layout is kept
// out of the interface. It is currently empty, as shaders only read vertex data.
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

   // Renderer internal.
   nri::Pipeline& GetNriPipeline() const;
   nri::PipelineLayout& GetNriPipelineLayout() const;

private:
   void Release();

   GraphicsDevice* m_Device = nullptr;
   nri::PipelineLayout* m_PipelineLayout = nullptr;
   nri::Pipeline* m_Pipeline = nullptr;
   uint32_t m_VertexStride = 0;
};

}
