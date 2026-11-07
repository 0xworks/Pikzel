#include "Pikzel/Renderer/CommandList.h"
#include "Pikzel/Renderer/Buffer.h"
#include "Pikzel/Renderer/DescriptorSet.h"
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


void CommandList::BeginRendering(const RenderPassSettings& settings) {
   assert(settings.colorTarget && "RenderPassSettings::colorTarget must be set");
   const RenderTarget& colorTarget = *settings.colorTarget;

   // The load operation says what happens to a target's existing contents at the start of
   // the pass: CLEAR replaces them, LOAD keeps them. Clearing is cheaper than it sounds, and
   // on tiled (mobile) GPUs it is cheaper than loading, because nothing is read from memory.
   nri::AttachmentDesc colorAttachment = {};
   colorAttachment.descriptor = &colorTarget.GetNriView();
   colorAttachment.storeOp = nri::StoreOp::STORE;
   if (settings.clearColor) {
      const Color& clearColor = *settings.clearColor;
      colorAttachment.loadOp = nri::LoadOp::CLEAR;
      colorAttachment.clearValue.color.f = {clearColor.r, clearColor.g, clearColor.b, clearColor.a};
   } else {
      colorAttachment.loadOp = nri::LoadOp::LOAD;
   }

   nri::RenderingDesc renderingDesc = {};
   renderingDesc.colors = &colorAttachment;
   renderingDesc.colorNum = 1;

   if (settings.depthTarget) {
      renderingDesc.depth.descriptor = &settings.depthTarget->GetNriView();
      renderingDesc.depth.storeOp = nri::StoreOp::STORE;
      if (settings.clearDepth) {
         renderingDesc.depth.loadOp = nri::LoadOp::CLEAR;
         renderingDesc.depth.clearValue.depthStencil = {.depth = *settings.clearDepth, .stencil = 0};
      } else {
         renderingDesc.depth.loadOp = nri::LoadOp::LOAD;
      }
   }

   m_Nri->CmdBeginRendering(*m_CommandBuffer, renderingDesc);

   // Viewport and scissor are dynamic state in NRI: they are set while recording rather
   // than baked into pipelines, so a resized window does not need new pipelines.
   SetViewport({.width = static_cast<float>(colorTarget.GetWidth()), .height = static_cast<float>(colorTarget.GetHeight())});
   SetScissor({.width = colorTarget.GetWidth(), .height = colorTarget.GetHeight()});
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


void CommandList::BindIndexBuffer(const Buffer& buffer, const IndexType indexType, const uint64_t offset) {
   const auto nriIndexType = indexType == IndexType::UInt16 ? nri::IndexType::UINT16 : nri::IndexType::UINT32;
   m_Nri->CmdSetIndexBuffer(*m_CommandBuffer, buffer.GetNriBuffer(), offset, nriIndexType);
   m_IsIndexBufferBound = true;
}


void CommandList::BindDescriptorSet(const uint32_t setIndex, const DescriptorSet& descriptorSet) {
   assert(m_BoundPipeline && "BindPipeline() must be called before BindDescriptorSet()");
   assert(setIndex < m_BoundPipeline->GetDescriptorSetLayouts().size() && "The bound pipeline has no descriptor set at this index");

   const nri::SetDescriptorSetDesc desc = {
      .setIndex = setIndex,
      .descriptorSet = &descriptorSet.GetNriDescriptorSet(),
      .bindPoint = nri::BindPoint::GRAPHICS,
   };
   m_Nri->CmdSetDescriptorSet(*m_CommandBuffer, desc);
}


void CommandList::PushConstants(const std::span<const std::byte> data) {
   assert(m_BoundPipeline && "BindPipeline() must be called before PushConstants()");
   assert(data.size() <= m_BoundPipeline->GetPushConstantsSize() && "More push constants than the pipeline's pushConstantsSize");

   const nri::SetRootConstantsDesc desc = {
      .rootConstantIndex = 0,
      .data = data.data(),
      .size = static_cast<uint32_t>(data.size()),
      .offset = 0,
      .bindPoint = nri::BindPoint::GRAPHICS,
   };
   m_Nri->CmdSetRootConstants(*m_CommandBuffer, desc);
}


void CommandList::Draw(const uint32_t vertexCount, const uint32_t instanceCount, const uint32_t firstVertex, const uint32_t firstInstance) {
   assert(m_BoundPipeline && "BindPipeline() must be called before Draw()");
   m_Nri->CmdDraw(*m_CommandBuffer, {vertexCount, instanceCount, firstVertex, firstInstance});
}


void CommandList::DrawIndexed(const uint32_t indexCount, const uint32_t instanceCount, const uint32_t firstIndex, const int32_t vertexOffset, const uint32_t firstInstance) {
   assert(m_BoundPipeline && "BindPipeline() must be called before DrawIndexed()");
   assert(m_IsIndexBufferBound && "BindIndexBuffer() must be called before DrawIndexed()");
   m_Nri->CmdDrawIndexed(*m_CommandBuffer, {indexCount, instanceCount, firstIndex, vertexOffset, firstInstance});
}

}
