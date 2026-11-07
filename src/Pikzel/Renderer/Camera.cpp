#include "Pikzel/Renderer/Camera.h"

#include <rtm/matrix4x4f.h>
#include <rtm/vector4f.h>

#include <cassert>
#include <cmath>

namespace Pikzel {

rtm::matrix4x4f CreateLookAtMatrix(const rtm::vector4f eye, const rtm::vector4f target, const rtm::vector4f up) {
   // The camera's axes in world space. The camera looks down its -Z axis, so its +Z axis
   // ("back") points away from the target.
   const rtm::vector4f forward = rtm::vector_normalize3(rtm::vector_sub(target, eye));
   const rtm::vector4f right = rtm::vector_normalize3(rtm::vector_cross3(forward, up));
   const rtm::vector4f cameraUp = rtm::vector_cross3(right, forward);
   const rtm::vector4f back = rtm::vector_neg(forward);
   assert(rtm::vector_is_finite3(right) && "up must not be parallel to the view direction");

   // The camera's transform would have these axes as its rows, and eye as its translation.
   // Its inverse, the view matrix, undoes the translation and then the rotation. The
   // rotation is orthonormal, so its inverse is its transpose: the axes become columns.
   // The translation row is the eye position expressed along each axis, negated.
   return rtm::matrix4x4f{
      rtm::vector_set(rtm::vector_get_x(right), rtm::vector_get_x(cameraUp), rtm::vector_get_x(back), 0.0f),
      rtm::vector_set(rtm::vector_get_y(right), rtm::vector_get_y(cameraUp), rtm::vector_get_y(back), 0.0f),
      rtm::vector_set(rtm::vector_get_z(right), rtm::vector_get_z(cameraUp), rtm::vector_get_z(back), 0.0f),
      rtm::vector_set(-rtm::vector_dot3(right, eye), -rtm::vector_dot3(cameraUp, eye), -rtm::vector_dot3(back, eye), 1.0f),
   };
}


rtm::matrix4x4f CreatePerspectiveMatrix(const float verticalFov, const float aspectRatio, const float nearDistance) {
   assert(verticalFov > 0.0f && verticalFov < 3.14159265f && "verticalFov must be between 0 and pi radians");
   assert(aspectRatio > 0.0f && "aspectRatio must be positive");
   assert(nearDistance > 0.0f && "nearDistance must be positive");

   // A view-space point (x, y, z) in front of the camera has z < 0. The matrix produces the
   // clip-space point
   //
   //    (x * f / aspectRatio,  y * f,  nearDistance,  -z)
   //
   // and the GPU then divides by the last component, w = -z (the distance in front of the
   // camera), which is what makes distant objects smaller. f scales the view so that the
   // edges of the field of view land on x, y = -1 and +1.
   //
   // After the divide, depth is nearDistance / -z: 1 at the near plane, approaching 0 as
   // the distance grows towards infinity. That is the reversed-Z mapping (see Camera.h).
   const float f = 1.0f / std::tan(verticalFov * 0.5f);

   // Rows, for p * M: each row is what one input component (x, y, z, w) contributes to the
   // output.
   return rtm::matrix4x4f{
      rtm::vector_set(f / aspectRatio, 0.0f, 0.0f, 0.0f),
      rtm::vector_set(0.0f, f, 0.0f, 0.0f),
      rtm::vector_set(0.0f, 0.0f, 0.0f, -1.0f),
      rtm::vector_set(0.0f, 0.0f, nearDistance, 0.0f),
   };
}

}
