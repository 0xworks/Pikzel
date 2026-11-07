#pragma once

#include "Pikzel/Renderer/Pipeline.h"

#include <cstdint>

namespace nri {
struct Descriptor;
struct DescriptorSet;
}

namespace Pikzel {

class Buffer;
class GraphicsDevice;
class Sampler;
class Texture;

// A group of resource bindings (constant buffers, textures, samplers) that shaders read,
// matching one DescriptorSetLayout of a pipeline.
//
// Shaders do not see resources directly. They see descriptors: small GPU-readable records
// that say where a resource is and how to interpret it. A descriptor set is a block of
// descriptor slots, laid out as its DescriptorSetLayout describes, which is bound with a
// single CommandList::BindDescriptorSet call. Grouping resources into sets lets a renderer
// bind everything a material needs at once, and leave per-frame data bound across draws.
//
// Lifetime and updates
// --------------------
// A set only refers to resources; the resources must stay alive while it is in use. The
// Set* functions write descriptors immediately, so they must not be called while a frame
// that uses the set may still be executing on the GPU. Sets that change every frame are
// therefore kept once per frame in flight, like the buffers they point to.
//
// Descriptor sets are allocated from the GraphicsDevice's descriptor pool and, as NRI
// works, are never returned to it individually: the pool is only reclaimed as a whole. So
// create sets up front (per material, per frame in flight) rather than per draw. A future
// renderer is likely to move to bindless resources, where shaders index one large array of
// descriptors and this kind of set is barely needed.
class DescriptorSet {
public:
   // Allocates a set matching pipeline's descriptor set layout number setIndex (an index
   // into GraphicsPipelineSettings::descriptorSets). The set can then be bound to any
   // pipeline with an identical layout in that position.
   DescriptorSet(GraphicsDevice& device, const Pipeline& pipeline, uint32_t setIndex);

   // Each function fills one descriptor of the range rangeIndex (an index into the
   // DescriptorSetLayout's ranges), at arrayIndex within the range. The range's type must
   // match: a ConstantBuffer range takes a buffer created with BufferUsage::Constant, and
   // so on.
   void SetConstantBuffer(uint32_t rangeIndex, const Buffer& buffer, uint32_t arrayIndex = 0);
   void SetTexture(uint32_t rangeIndex, const Texture& texture, uint32_t arrayIndex = 0);
   void SetSampler(uint32_t rangeIndex, const Sampler& sampler, uint32_t arrayIndex = 0);

   // Renderer internal.
   nri::DescriptorSet& GetNriDescriptorSet() const;

private:
   void Update(uint32_t rangeIndex, DescriptorType type, nri::Descriptor& descriptor, uint32_t arrayIndex);

   GraphicsDevice* m_Device = nullptr;

   // A copy of the layout, so that Set* calls can be checked against it without the set
   // depending on the pipeline staying alive.
   DescriptorSetLayout m_Layout;

   // Owned by the device's descriptor pool, not by this object.
   nri::DescriptorSet* m_DescriptorSet = nullptr;
};

}
