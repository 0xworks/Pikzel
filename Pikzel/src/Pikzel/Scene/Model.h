#pragma once

#include "Mesh.h"

#include <glm/glm.hpp>

#include <utility>
#include <vector>

namespace Pikzel {

   // Right now, a "model" is just a collection of meshes, that's it.
   struct Model {

      ~Model() {
         ;
      }

      std::pair<glm::vec3, glm::vec3> AABB = {glm::vec3{FLT_MAX}, glm::vec3{-FLT_MAX}};
      std::vector<Mesh> Meshes;
   };

}
