#pragma once

#include "Pikzel/Events/ApplicationEvents.h"
#include "Pikzel/Input/Input.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>


// Quick and dirty camera class.
// this is not the final product!

struct Camera {

   glm::vec3 Position {0.0f, 0.0f, 0.0f};
   glm::vec3 Direction = {0.0f, 0.0f, -1.0f};
   glm::vec3 UpVector = {0.0f, 1.0f, 0.0f};
   float FoVRadians = glm::radians(45.0f);

   glm::mat4 Projection = glm::identity<glm::mat4>();

   void Update(const Pikzel::Input& input, const Pikzel::DeltaTime deltaTime);

};
