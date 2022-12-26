#pragma once

#include "Pikzel/Core/Core.h"

#include <glm/ext/vector_float3.hpp>
#include <glm/ext/quaternion_float.hpp>

namespace Pikzel {

   struct PKZL_API Transform {
      glm::vec3 Translation;
      glm::quat Rotation;
      glm::vec3 RotationEuler;
      glm::vec3 Scale;
   };


}
