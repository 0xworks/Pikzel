#pragma once

namespace Pikzel {

   struct DirectionalLight {
      alignas(16) glm::vec3 Direction;
      alignas(16) glm::vec3 Color;
      alignas(16) glm::vec3 Ambient;
   };


   struct PointLight {
      alignas(16) glm::vec3 Position;
      alignas(16) glm::vec3 Color;
      alignas(4) float Constant;
      alignas(4) float Linear;
      alignas(4) float Quadratic;
   };

}
