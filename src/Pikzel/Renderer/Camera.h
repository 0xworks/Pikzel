#pragma once

#include <rtm/types.h>

// View and projection matrices, following the engine's coordinate conventions.
//
// Coordinate system
// -----------------
// World and view space are right-handed with +Y up, and a camera looks down its -Z axis,
// with +X to its right. These are glTF's conventions, the engine's main asset format, so
// imported scenes need no conversion. (RTM's own camera helpers are left-handed, looking
// down +Z, which is why the engine does not use them.)
//
// Clip space follows D3D: x and y from -1 to +1 with +y up, and depth from 0 to 1. NRI
// adjusts Vulkan's viewport so that the same holds there.
//
// Reversed-Z
// ----------
// The projection maps the near plane to depth 1 and infinitely distant points to depth 0:
// the reverse of the traditional mapping. Perspective projection makes depth proportional
// to 1/distance, which crowds most of the depth range close to the near plane. Floating
// point numbers are also unevenly spread, with far more representable values near 0 than
// near 1. With traditional depth these two effects pile up at the near plane and leave
// distant geometry with very little precision, causing z-fighting. Reversing the range
// makes them roughly cancel out, giving almost uniform precision at every distance with a
// 32-bit float depth buffer. Precision is good enough that the far plane can be at
// infinity, which removes a parameter that otherwise needs tuning per scene.
//
// The consequences elsewhere are small: the depth buffer is cleared to 0 rather than 1, and
// the depth test passes for greater values rather than smaller ones (CompareOp::Greater,
// the default in GraphicsPipelineSettings).
//
// Matrices use RTM's row-vector convention: p' = p * M, so a world point is taken to clip
// space with p * view * projection (see ShaderTypes.h for how this carries into shaders).
namespace Pikzel {

// Returns the view matrix of a camera at eye, looking at target, with the given up
// direction (usually +Y). The view matrix takes world space to view space; it is the
// inverse of the camera's own transform. up must not be parallel to the view direction.
rtm::matrix4x4f CreateLookAtMatrix(rtm::vector4f eye, rtm::vector4f target, rtm::vector4f up);

// Returns a reversed-Z perspective projection with an infinite far plane.
// verticalFov is the full vertical field of view, in radians; aspectRatio is width / height.
rtm::matrix4x4f CreatePerspectiveMatrix(float verticalFov, float aspectRatio, float nearDistance);

}
