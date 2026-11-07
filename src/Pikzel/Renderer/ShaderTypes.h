#pragma once

#include <rtm/types.h>

#include <array>

// C++ types that mirror Slang types in GPU buffers (constant buffers, push constants).
//
// CPU math types cannot simply be copied into GPU buffers. Their size, alignment and
// element order are chosen for the CPU (RTM's types are SIMD registers, for example), while
// the GPU reads data according to the shader language's layout rules. Data shared with
// shaders is therefore declared with these plain types, whose layout is fixed and checked,
// and converted from RTM explicitly.
//
// Matrix convention
// -----------------
// RTM uses row vectors: a point is transformed as p' = p * M, and a matrix stores its rows
// (x_axis, y_axis, z_axis, w_axis) one after another, the translation in the last row.
// Engine shaders follow the same convention, so a matrix means the same thing on both
// sides: shaders transform with mul(vector, matrix), and are compiled with row-major matrix
// layout (-matrix-layout-row-major, see cmake/Shaders.cmake), so a float4x4 is read from
// memory one row at a time. Float4x4 stores the rows in that order, and no transposes are
// needed anywhere.
//
// Packing rules
// -------------
// Constant buffers and push constants follow HLSL's constant buffer packing: members are
// laid out in order, in 16-byte rows, and a member may not straddle a row boundary. A
// float3 followed by a float packs into one row; a float3 followed by another float3 does
// not, and the second starts a new row. Structs that mirror shader data should
// static_assert their size and member offsets, so that a mismatch is a compile error rather
// than garbage on screen.
namespace Pikzel {

struct Float4x4 {
   // Rows one after another: elements[row * 4 + column].
   std::array<float, 16> elements = {};
};

static_assert(sizeof(Float4x4) == 64);

Float4x4 ToFloat4x4(const rtm::matrix4x4f& matrix);

}
