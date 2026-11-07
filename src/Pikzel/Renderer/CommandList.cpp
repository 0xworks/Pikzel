#include "Pikzel/Renderer/CommandList.h"
#include "Pikzel/Renderer/Buffer.h"
#include "Pikzel/Renderer/Nri.h"
#include "Pikzel/Renderer/Pipeline.h"

#include <cassert>

namespace Pikzel {

RenderTarget::RenderTarget(nri::Descriptor& view, const uint32_t width, const uint32_t height)
: m_View(&view), m_Width(width), m_Height(height) {}


uint32_t RenderTarget::GetWidth() const {
   return m_Width;
}


uint32_t RenderTarget::GetHeight() const {
   return m_Height;
}


nri::Descriptor& RenderTarget::GetNriView() const {
   return *m_View;
}


CommandList::CommandList(const NriInterface& nri, nri::CommandBuffer& commandBuffer)
: m_Nri(&nri), m_CommandBuffer(&commandBuffer) {}


void CommandList::BeginRendering(const RenderTarget& target, const std::optional<Color>& clearColor) {
   nri::AttachmentDesc colorAttachment = {};
   colorAttachment.descriptor = &target.GetNriView();
   colorAttachment.storeOp = nri::StoreOp::STORE;
   if (clearColor) {
      colorAttachment.loadOp = nri::LoadOp::CLEAR;
      colorAttachment.clearValue.color.f = {clearColor->r, clearColor->g, clearColor->b, clearColor->a};
   } else {
      colorAttachment.loadOp = nri::LoadOp::LOAD;
   }

   nri::RenderingDesc renderingDesc = {};
   renderingDesc.colors = &colorAttachment;
   renderingDesc.colorNum = 1;
   m_Nri->CmdBeginRendering(*m_CommandBuffer, renderingDesc);

   // Viewport and scissor are dynamic state in NRI: they are set while recording rather
   // than baked into pipelines, so a resized window does not need new pipelines.
   SetViewport({.width = static_cast<float>(target.GetWidth()), .height = static_cast<float>(target.GetHeight())});
   SetScissor({.width = target.GetWidth(), .height = target.GetHeight()});
}


void CommandList::EndRendering() {
   m_Nri->CmdEndRendering(*m_CommandBuffer);
   m_BoundPipeline = nullptr;
}


void CommandList::SetViewport(const Viewport& viewport) {
   const nri::Viewport nriViewport = {viewport.x, viewport.y, viewport.width, viewport.height, viewport.minDepth, viewport.maxDepth};
   m_Nri->CmdSetViewports(*m_CommandBuffer, &nriViewport, 1);
}


void CommandList::SetScissor(const ScissorRect& scissor) {
   const nri::Rect rect = {static_cast<int16_t>(scissor.x), static_cast<int16_t>(scissor.y), static_cast<nri::Dim_t>(scissor.width), static_cast<nri::Dim_t>(scissor.height)};
   m_Nri->CmdSetScissors(*m_CommandBuffer, &rect, 1);
}


void CommandList::BindPipeline(const Pipeline& pipeline) {
   // NRI binds the layout (which resources the shaders expect) separately from the
   // pipeline itself. Every engine pipeline owns its layout, so both are bound here.
   m_Nri->CmdSetPipelineLayout(*m_CommandBuffer, nri::BindPoint::GRAPHICS, pipeline.GetNriPipelineLayout());
   m_Nri->CmdSetPipeline(*m_CommandBuffer, pipeline.GetNriPipeline());
   m_BoundPipeline = &pipeline;
}


void CommandList::BindVertexBuffer(const Buffer& buffer, const uint64_t offset) {
   assert(m_BoundPipeline && "BindPipeline() must be called before BindVertexBuffer()");

   const nri::VertexBufferDesc vertexBuffer = {
      .buffer = &buffer.GetNriBuffer(),
      .offset = offset,
      .stride = m_BoundPipeline->GetVertexStride(),
   };
   m_Nri->CmdSetVertexBuffers(*m_CommandBuffer, 0, &vertexBuffer, 1);
}


void CommandList::Draw(const uint32_t vertexCount, const uint32_t instanceCount, const uint32_t firstVertex, const uint32_t firstInstance) {
   assert(m_BoundPipeline && "BindPipeline() must be called before Draw()");
   m_Nri->CmdDraw(*m_CommandBuffer, {vertexCount, instanceCount, firstVertex, firstInstance});
}

}
