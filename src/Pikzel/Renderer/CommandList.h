#pragma once

#include "Pikzel/Renderer/Color.h"

#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <type_traits>

namespace nri {
struct CommandBuffer;
struct Descriptor;
}

namespace Pikzel {

class Buffer;
class DescriptorSet;
class Pipeline;
struct NriInterface;

// A texture that can be rendered into (a color target or a depth buffer), together with
// its size. SwapChain::BeginFrame() hands out the back buffer and depth buffer as render
// targets, and Texture::GetRenderTarget() makes one from a texture.
//
// A RenderTarget is a lightweight, non-owning view: the texture must outlive it.
class RenderTarget {
public:
   RenderTarget(nri::Descriptor& view, uint32_t width, uint32_t height);

   uint32_t GetWidth() const;
   uint32_t GetHeight() const;

   // Renderer internal.
   nri::Descriptor& GetNriView() const;

private:
   nri::Descriptor* m_View = nullptr;
   uint32_t m_Width = 0;
   uint32_t m_Height = 0;
};

// The region of the render target that clip space (-1..+1) maps onto. The origin is the
// top-left corner, with y increasing downwards.
struct Viewport {
   float x = 0.0f;
   float y = 0.0f;
   float width = 0.0f;
   float height = 0.0f;
   float minDepth = 0.0f;
   float maxDepth = 1.0f;
};

// Pixels outside this rectangle are discarded.
struct ScissorRect {
   int32_t x = 0;
   int32_t y = 0;
   uint32_t width = 0;
   uint32_t height = 0;
};

// What a rendering pass draws into. A target with a clear value is cleared when the pass
// begins; one without keeps its existing contents.
//
// The clear color is linear, like every Color; it is encoded for display along with
// everything else drawn to an sRGB target. With the engine's reversed-Z convention (see
// Camera.h) the far plane is at depth 0, so a depth buffer is normally cleared to 0.
struct RenderPassSettings {
   const RenderTarget* colorTarget = nullptr;
   std::optional<Color> clearColor;

   const RenderTarget* depthTarget = nullptr;
   std::optional<float> clearDepth;
};

// The size of each index in an index buffer. 16-bit indices halve the memory and bandwidth
// of 32-bit ones, and suffice for meshes of up to 65536 vertices.
enum class IndexType : uint8_t {
   UInt16,
   UInt32,
};

// Records GPU commands into a command buffer.
//
// Recording only stores commands: nothing executes until the command buffer is submitted,
// which SwapChain::EndFrame() does. A CommandList is a lightweight view of a command
// buffer owned elsewhere (by SwapChain, for a frame's commands), so it is cheap to copy
// and must not outlive that owner.
//
// Drawing happens inside a rendering pass, between BeginRendering() and EndRendering().
// Within a pass, a pipeline must be bound before anything that depends on its layout:
// vertex buffers, descriptor sets, push constants and draws.
class CommandList {
public:
   CommandList(const NriInterface& nri, nri::CommandBuffer& commandBuffer);

   // Starts a rendering pass. settings.colorTarget is required. The viewport and scissor
   // rectangle are set to cover the whole color target, which is what most passes want.
   void BeginRendering(const RenderPassSettings& settings);
   void EndRendering();

   void SetViewport(const Viewport& viewport);
   void SetScissor(const ScissorRect& scissor);

   void BindPipeline(const Pipeline& pipeline);

   // Binds vertex data for the bound pipeline. The vertex stride comes from the pipeline.
   void BindVertexBuffer(const Buffer& buffer, uint64_t offset = 0);

   // Binds the index buffer used by DrawIndexed().
   void BindIndexBuffer(const Buffer& buffer, IndexType indexType, uint64_t offset = 0);

   // Binds a descriptor set to position setIndex of the bound pipeline's layout (an index
   // into GraphicsPipelineSettings::descriptorSets). Binding a different pipeline unbinds
   // all descriptor sets, so they must be bound again after it.
   void BindDescriptorSet(uint32_t setIndex, const DescriptorSet& descriptorSet);

   // Sets the bound pipeline's push constants. The data is copied into the command buffer
   // immediately, so it can be changed (or go out of scope) straight after this call, and
   // each draw sees the values pushed most recently before it.
   void PushConstants(std::span<const std::byte> data);

   template<typename T>
   void PushConstants(const T& data) {
      static_assert(std::is_trivially_copyable_v<T>, "Push constants are copied as raw bytes");
      PushConstants(std::as_bytes(std::span(&data, 1)));
   }

   void Draw(uint32_t vertexCount, uint32_t instanceCount = 1, uint32_t firstVertex = 0, uint32_t firstInstance = 0);

   // Draws using the bound index buffer: each index selects a vertex, so vertices shared by
   // several triangles are stored (and usually transformed) once. vertexOffset is added to
   // every index before the vertex is fetched.
   void DrawIndexed(uint32_t indexCount, uint32_t instanceCount = 1, uint32_t firstIndex = 0, int32_t vertexOffset = 0, uint32_t firstInstance = 0);

private:
   const NriInterface* m_Nri = nullptr;
   nri::CommandBuffer* m_CommandBuffer = nullptr;
   const Pipeline* m_BoundPipeline = nullptr;
   bool m_IsIndexBufferBound = false;
};

}
