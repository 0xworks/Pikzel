#pragma once

#include "Pikzel/Renderer/Color.h"

#include <cstdint>
#include <optional>

namespace nri {
struct CommandBuffer;
struct Descriptor;
}

namespace Pikzel {

class Buffer;
class Pipeline;
struct NriInterface;

// A texture that can be rendered into, together with its size. For now the only render
// targets are swap chain back buffers, which SwapChain::BeginFrame() hands out.
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

// Records GPU commands into a command buffer.
//
// Recording only stores commands: nothing executes until the command buffer is submitted,
// which SwapChain::EndFrame() does. A CommandList is a lightweight view of a command
// buffer owned elsewhere (by SwapChain, for a frame's commands), so it is cheap to copy
// and must not outlive that owner.
//
// Drawing happens inside a rendering pass, between BeginRendering() and EndRendering().
// Within a pass, a pipeline must be bound before vertex buffers and draws.
class CommandList {
public:
   CommandList(const NriInterface& nri, nri::CommandBuffer& commandBuffer);

   // Starts rendering into target. If clearColor is given, the target is cleared to it;
   // otherwise its existing contents are kept. Like every Color, clearColor is linear; it
   // is encoded for display along with everything else drawn to an sRGB target.
   //
   // The viewport and scissor rectangle are set to cover the whole target, which is what
   // most passes want.
   void BeginRendering(const RenderTarget& target, const std::optional<Color>& clearColor = std::nullopt);
   void EndRendering();

   void SetViewport(const Viewport& viewport);
   void SetScissor(const ScissorRect& scissor);

   void BindPipeline(const Pipeline& pipeline);

   // Binds vertex data for the bound pipeline. The vertex stride comes from the pipeline.
   void BindVertexBuffer(const Buffer& buffer, uint64_t offset = 0);

   void Draw(uint32_t vertexCount, uint32_t instanceCount = 1, uint32_t firstVertex = 0, uint32_t firstInstance = 0);

private:
   const NriInterface* m_Nri = nullptr;
   nri::CommandBuffer* m_CommandBuffer = nullptr;
   const Pipeline* m_BoundPipeline = nullptr;
};

}
