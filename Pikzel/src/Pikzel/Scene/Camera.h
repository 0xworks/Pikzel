#pragma once

#include "Pikzel/Events/ApplicationEvents.h"
#include "Pikzel/Input/Input.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>


// Quick and dirty camera class.
// this is not the final product!

struct PKZL_API Camera {

   glm::vec3 position {0.0f, 0.0f, 0.0f};
   glm::vec3 direction = {0.0f, 0.0f, -1.0f};
   glm::vec3 upVector = {0.0f, 1.0f, 0.0f};
   float fovRadians = glm::radians(45.0f);
   float moveSpeed = 2.5f; // TODO: should be in camera controller
   float rotateSpeed = 2.5f;

   glm::mat4 projection = glm::identity<glm::mat4>();

   void Update(const Pikzel::Input& input, const Pikzel::DeltaTime deltaTime);

};
