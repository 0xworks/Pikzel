#pragma once

#include "Pikzel/Core/Core.h"

namespace Pikzel {

   struct PKZL_API DirectionalLight {
      alignas(16) glm::vec3 direction = {};
      alignas(16) glm::vec3 color = {};
      alignas(16) glm::vec3 ambient = {};
      alignas(4) float size = 0.02;
   };


   struct PKZL_API PointLight {
      alignas(16) glm::vec3 position = {};
      alignas(16) glm::vec3 color = {};
      alignas(4) float size = 0.02;
      alignas(4) float power = 0.0;
   };

}
