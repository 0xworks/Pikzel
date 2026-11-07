#pragma once

#include <cstdint>

namespace Pikzel {

// Data formats for textures, render targets and vertex attributes.
//
// This is deliberately a small subset of what GPUs support: formats are added when the
// engine needs them. Names follow the usual graphics convention of channels, then bits
// per channel, then how the bits are interpreted:
//  - Float: IEEE floating point
//  - Unorm: unsigned integer mapped to [0, 1]
//  - Srgb:  as Unorm, but the GPU converts from sRGB to linear when reading and back
//           when writing
enum class Format : uint8_t {
   Unknown,

   // Typically vertex attributes
   R32Float,
   RG32Float,
   RGB32Float,
   RGBA32Float,

   // Typically textures and render targets
   RGBA8Unorm,
   RGBA8Srgb,
   BGRA8Unorm,
   BGRA8Srgb,
   RGB10A2Unorm,
   RGBA16Float,
};

}
