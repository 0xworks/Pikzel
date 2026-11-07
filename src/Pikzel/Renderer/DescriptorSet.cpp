#include "Pikzel/Renderer/DescriptorSet.h"
#include "Pikzel/Renderer/Buffer.h"
#include "Pikzel/Renderer/GraphicsDevice.h"
#include "Pikzel/Renderer/Nri.h"
#include "Pikzel/Renderer/Sampler.h"
#include "Pikzel/Renderer/Texture.h"

#include <format>
#include <stdexcept>

namespace Pikzel {

DescriptorSet::DescriptorSet(GraphicsDevice& device, const Pipeline& pipeline, const uint32_t setIndex)
: m_Device(&device) {
   const auto& layouts = pipeline.GetDescriptorSetLayouts();
   if (setIndex >= layouts.size()) {
      throw std::invalid_argument(std::format("The pipeline has {} descriptor sets, so there is no set {}", layouts.size(), setIndex));
   }
   m_Layout = layouts[setIndex];

   // The set's layout comes from the pipeline layout, which already describes it.
   CheckNri(device.GetNri().AllocateDescriptorSets(device.GetDescriptorPool(), pipeline.GetNriPipelineLayout(), setIndex, &m_DescriptorSet, 1, 0), "AllocateDescriptorSets");
}


void DescriptorSet::SetConstantBuffer(const uint32_t rangeIndex, const Buffer& buffer, const uint32_t arrayIndex) {
   Update(rangeIndex, DescriptorType::ConstantBuffer, buffer.GetNriConstantBufferView(), arrayIndex);
}


void DescriptorSet::SetTexture(const uint32_t rangeIndex, const Texture& texture, const uint32_t arrayIndex) {
   Update(rangeIndex, DescriptorType::Texture, texture.GetNriShaderResourceView(), arrayIndex);
}


void DescriptorSet::SetSampler(const uint32_t rangeIndex, const Sampler& sampler, const uint32_t arrayIndex) {
   Update(rangeIndex, DescriptorType::Sampler, sampler.GetNriSampler(), arrayIndex);
}


nri::DescriptorSet& DescriptorSet::GetNriDescriptorSet() const {
   return *m_DescriptorSet;
}


// Copies the descriptor (a texture view, buffer view or sampler, created by the resource
// itself) into a slot of the set.
void DescriptorSet::Update(const uint32_t rangeIndex, const DescriptorType type, nri::Descriptor& descriptor, const uint32_t arrayIndex) {
   if (rangeIndex >= m_Layout.ranges.size()) {
      throw std::invalid_argument(std::format("The descriptor set has {} ranges, so there is no range {}", m_Layout.ranges.size(), rangeIndex));
   }
   const auto& range = m_Layout.ranges[rangeIndex];
   if (range.type != type) {
      throw std::invalid_argument(std::format("Descriptor range {} holds a different type of resource", rangeIndex));
   }
   if (arrayIndex >= range.count) {
      throw std::invalid_argument(std::format("Descriptor range {} has {} descriptors, so there is no index {}", rangeIndex, range.count, arrayIndex));
   }

   nri::Descriptor* const descriptors[] = {&descriptor};
   const nri::UpdateDescriptorRangeDesc update = {
      .descriptorSet = m_DescriptorSet,
      .rangeIndex = rangeIndex,
      .baseDescriptor = arrayIndex,
      .descriptors = descriptors,
      .descriptorNum = 1,
   };
   m_Device->GetNri().UpdateDescriptorRanges(&update, 1);
}

}
