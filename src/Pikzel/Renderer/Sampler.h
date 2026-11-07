#pragma once

#include <cstdint>

namespace nri {
struct Descriptor;
}

namespace Pikzel {

class GraphicsDevice;

// How texels are combined when a texture is sampled between texel centres, or at a
// different size from the one it is displayed at.
enum class Filter : uint8_t {
   // The closest texel: blocky when magnified, but exact. Suits pixel art and lookup tables.
   Nearest,
   // A weighted average of the nearest texels (bilinear, and trilinear between mip levels).
   Linear,
};

// What texture coordinates outside [0, 1] read.
enum class AddressMode : uint8_t {
   // The texture tiles.
   Repeat,
   // The texture tiles, flipping every other copy.
   MirroredRepeat,
   // The edge texels extend outwards.
   ClampToEdge,
};

struct SamplerSettings {
   Filter filter = Filter::Linear;
   AddressMode addressMode = AddressMode::Repeat;

   // Anisotropic filtering keeps textures sharp when seen at a glancing angle, by taking up
   // to this many samples along the direction the texture is stretched. 1 disables it; 16 is
   // the usual hardware maximum.
   uint32_t maxAnisotropy = 1;
};

// The state that controls how shaders read textures: filtering and addressing.
//
// Samplers are separate objects from textures, in Slang (SamplerState) as in Vulkan and
// D3D12, so a handful of samplers can serve every texture in a scene. Like a texture view,
// a sampler is a descriptor, and shaders reach it through a DescriptorSet.
class Sampler {
public:
   Sampler(GraphicsDevice& device, const SamplerSettings& settings = {});
   ~Sampler();

   Sampler(const Sampler&) = delete;
   Sampler& operator=(const Sampler&) = delete;
   Sampler(Sampler&& other) noexcept;
   Sampler& operator=(Sampler&& other) noexcept;

   // Renderer internal.
   nri::Descriptor& GetNriSampler() const;

private:
   void Release();

   GraphicsDevice* m_Device = nullptr;
   nri::Descriptor* m_Sampler = nullptr;
};

}
