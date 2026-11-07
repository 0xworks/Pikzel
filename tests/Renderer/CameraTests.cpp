#include "Pikzel/Renderer/Camera.h"

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include <rtm/matrix4x4f.h>
#include <rtm/vector4f.h>

#include <cmath>
#include <numbers>

namespace pk = Pikzel;

using Catch::Matchers::WithinAbs;

namespace {

constexpr float s_Tolerance = 1e-5f;

// Transforms a point with RTM's row-vector convention, p * M, as shaders do.
rtm::vector4f TransformPoint(const float x, const float y, const float z, const rtm::matrix4x4f& matrix) {
   return rtm::matrix_mul_vector(rtm::vector_set(x, y, z, 1.0f), matrix);
}


void CheckVector(const rtm::vector4f& actual, const float x, const float y, const float z, const float w) {
   CHECK_THAT(rtm::vector_get_x(actual), WithinAbs(x, s_Tolerance));
   CHECK_THAT(rtm::vector_get_y(actual), WithinAbs(y, s_Tolerance));
   CHECK_THAT(rtm::vector_get_z(actual), WithinAbs(z, s_Tolerance));
   CHECK_THAT(rtm::vector_get_w(actual), WithinAbs(w, s_Tolerance));
}

}


TEST_CASE("Look-at view matrix follows the right-handed, -Z forward convention", "[renderer][camera]") {
   const auto eye = rtm::vector_set(1.0f, 2.0f, 5.0f);
   const auto target = rtm::vector_set(1.0f, 2.0f, 0.0f);
   const auto up = rtm::vector_set(0.0f, 1.0f, 0.0f);
   const auto view = pk::CreateLookAtMatrix(eye, target, up);

   SECTION("the eye is at the view-space origin") {
      CheckVector(TransformPoint(1.0f, 2.0f, 5.0f, view), 0.0f, 0.0f, 0.0f, 1.0f);
   }

   SECTION("the target is straight ahead, along -Z") {
      CheckVector(TransformPoint(1.0f, 2.0f, 0.0f, view), 0.0f, 0.0f, -5.0f, 1.0f);
   }

   SECTION("world up stays up, and world +X is to the right") {
      CheckVector(TransformPoint(1.0f, 3.0f, 5.0f, view), 0.0f, 1.0f, 0.0f, 1.0f);
      CheckVector(TransformPoint(2.0f, 2.0f, 5.0f, view), 1.0f, 0.0f, 0.0f, 1.0f);
   }
}


TEST_CASE("Look-at view matrix handles an arbitrary viewing direction", "[renderer][camera]") {
   // Looking from +X towards the origin, the camera's right is world -Z.
   const auto view = pk::CreateLookAtMatrix(rtm::vector_set(4.0f, 0.0f, 0.0f), rtm::vector_zero(), rtm::vector_set(0.0f, 1.0f, 0.0f));
   CheckVector(TransformPoint(0.0f, 0.0f, 0.0f, view), 0.0f, 0.0f, -4.0f, 1.0f);
   CheckVector(TransformPoint(4.0f, 0.0f, -1.0f, view), 1.0f, 0.0f, 0.0f, 1.0f);
}


TEST_CASE("Perspective projection is reversed-Z with an infinite far plane", "[renderer][camera]") {
   constexpr float verticalFov = std::numbers::pi_v<float> / 2.0f; // 90 degrees: tan(fov / 2) = 1
   constexpr float aspectRatio = 2.0f;
   constexpr float nearDistance = 0.5f;
   const auto projection = pk::CreatePerspectiveMatrix(verticalFov, aspectRatio, nearDistance);

   // Depth after the perspective divide.
   const auto depthAt = [&](const float distance) {
      const auto clip = TransformPoint(0.0f, 0.0f, -distance, projection);
      return rtm::vector_get_z(clip) / rtm::vector_get_w(clip);
   };

   SECTION("w is the distance in front of the camera") {
      CHECK_THAT(rtm::vector_get_w(TransformPoint(0.0f, 0.0f, -7.0f, projection)), WithinAbs(7.0f, s_Tolerance));
   }

   SECTION("the near plane has depth 1") {
      CHECK_THAT(depthAt(nearDistance), WithinAbs(1.0f, s_Tolerance));
   }

   SECTION("depth decreases with distance, towards 0") {
      CHECK(depthAt(1.0f) < depthAt(nearDistance));
      CHECK(depthAt(100.0f) < depthAt(1.0f));
      CHECK(depthAt(1.0e6f) > 0.0f);
      CHECK_THAT(depthAt(1.0e6f), WithinAbs(0.0f, 1e-6f));
   }

   SECTION("the edges of the field of view map to the edges of clip space") {
      // At distance d, a 90 degree vertical field of view spans y = -d to +d, and the 2:1
      // aspect ratio makes it span x = -2d to +2d.
      const auto top = TransformPoint(0.0f, 3.0f, -3.0f, projection);
      CHECK_THAT(rtm::vector_get_y(top) / rtm::vector_get_w(top), WithinAbs(1.0f, s_Tolerance));

      const auto right = TransformPoint(6.0f, 0.0f, -3.0f, projection);
      CHECK_THAT(rtm::vector_get_x(right) / rtm::vector_get_w(right), WithinAbs(1.0f, s_Tolerance));
   }
}
