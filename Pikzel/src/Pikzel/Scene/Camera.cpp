#include "Camera.h"

#include "Pikzel/Input/MouseButtons.h"

#include <glm/gtx/rotate_vector.hpp>

// TODO: this should be in a camera controller...
void Camera::Update(const Pikzel::Input& input, const Pikzel::DeltaTime deltaTime) {

   float dx = input.GetAxis("X"_hs) * deltaTime.count() * MoveSpeed;
   float dy = input.GetAxis("Y"_hs) * deltaTime.count() * MoveSpeed;
   float dz = input.GetAxis("Z"_hs) * deltaTime.count() * MoveSpeed;

   Position += dx * glm::normalize(glm::cross(Direction, UpVector));
   Position += dy * UpVector;
   Position += dz * Direction;

   if (input.IsMouseButtonPressed(Pikzel::MouseButton::Right)) {
      float dYawRadians = glm::radians(input.GetAxis("MouseX"_hs) * deltaTime.count() * RotateSpeed);
      float dPitchRadians = glm::radians(input.GetAxis("MouseY"_hs) * deltaTime.count() * RotateSpeed);

      Direction = glm::rotate(Direction, -dYawRadians, UpVector);
      glm::vec3 right = glm::cross(Direction, UpVector);
      Direction = glm::rotate(Direction, dPitchRadians, right);
   }
}
