#pragma once

#include "Pikzel/Core/Core.h"

namespace Pikzel {

   struct PKZL_API DirectionalLight {
      alignas(16) glm::vec3 Direction = {};
      alignas(16) glm::vec3 Color = {};
      alignas(16) glm::vec3 Ambient = {};
      alignas(4) float Size = 0.02;
   };


   struct PKZL_API PointLight {
      alignas(16) glm::vec3 Position = {};
      alignas(16) glm::vec3 Color = {};
      alignas(4) float Size = 0.02;
      alignas(4) float Power = 0.0;
   };

}
