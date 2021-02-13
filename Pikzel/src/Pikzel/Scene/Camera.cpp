#include "Camera.h"

#include "Pikzel/Input/MouseButtons.h"

#include <glm/gtx/rotate_vector.hpp>

// TODO: this should be in a camera controller...
void Camera::Update(const Pikzel::Input& input, const Pikzel::DeltaTime deltaTime) {

   float dx = input.GetAxis("X"_hs) * deltaTime.count() * moveSpeed;
   float dy = input.GetAxis("Y"_hs) * deltaTime.count() * moveSpeed;
   float dz = input.GetAxis("Z"_hs) * deltaTime.count() * moveSpeed;

   position += dx * glm::normalize(glm::cross(direction, upVector));
   position += dy * upVector;
   position += dz * direction;

   if (input.IsMouseButtonPressed(Pikzel::MouseButton::Right)) {
      float dYawRadians = glm::radians(input.GetAxis("MouseX"_hs) * deltaTime.count() * rotateSpeed);
      float dPitchRadians = glm::radians(input.GetAxis("MouseY"_hs) * deltaTime.count() * rotateSpeed);

      direction = glm::rotate(direction, -dYawRadians, upVector);
      glm::vec3 right = glm::cross(direction, upVector);
      direction = glm::rotate(direction, dPitchRadians, right);
   }
}
