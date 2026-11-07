#include "Pikzel/Renderer/ShaderTypes.h"

#include <catch2/catch_test_macros.hpp>

#include <rtm/matrix4x4f.h>
#include <rtm/vector4f.h>

namespace pk = Pikzel;

TEST_CASE("Float4x4 stores RTM matrices row by row", "[renderer][shadertypes]") {
   // RTM's axes are the rows of a row-vector matrix; w_axis holds the translation.
   const rtm::matrix4x4f matrix{
      rtm::vector_set(1.0f, 2.0f, 3.0f, 4.0f),
      rtm::vector_set(5.0f, 6.0f, 7.0f, 8.0f),
      rtm::vector_set(9.0f, 10.0f, 11.0f, 12.0f),
      rtm::vector_set(13.0f, 14.0f, 15.0f, 16.0f),
   };

   const pk::Float4x4 result = pk::ToFloat4x4(matrix);
   for (int i = 0; i < 16; ++i) {
      CHECK(result.elements[i] == static_cast<float>(i + 1));
   }
}
