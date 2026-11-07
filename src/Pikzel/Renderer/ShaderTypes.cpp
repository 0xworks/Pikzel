#include "Pikzel/Renderer/ShaderTypes.h"

#include <rtm/vector4f.h>

namespace Pikzel {

Float4x4 ToFloat4x4(const rtm::matrix4x4f& matrix) {
   Float4x4 result;
   rtm::vector_store(matrix.x_axis, &result.elements[0]);
   rtm::vector_store(matrix.y_axis, &result.elements[4]);
   rtm::vector_store(matrix.z_axis, &result.elements[8]);
   rtm::vector_store(matrix.w_axis, &result.elements[12]);
   return result;
}

}
