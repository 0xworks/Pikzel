#include "Camera.h"

#include "Pikzel/Input/MouseButtons.h"

#include <glm/gtx/rotate_vector.hpp>

void Camera::Update(const Pikzel::Input& input, const Pikzel::DeltaTime deltaTime) {
   constexpr float cameraSpeed = 2.5f;

   float dx = input.GetAxis("X"_hs) * deltaTime.count() * cameraSpeed;
   float dy = input.GetAxis("Y"_hs) * deltaTime.count() * cameraSpeed;
   float dz = input.GetAxis("Z"_hs) * deltaTime.count() * cameraSpeed;

   Position += dx * glm::normalize(glm::cross(Direction, UpVector));
   Position += dy * UpVector;
   Position += dz * Direction;

   if (input.IsMouseButtonPressed(Pikzel::MouseButton::Right)) {
      float dYawRadians = glm::radians(input.GetAxis("MouseX"_hs) * deltaTime.count() * cameraSpeed);
      float dPitchRadians = glm::radians(input.GetAxis("MouseY"_hs) * deltaTime.count() * cameraSpeed);

      Direction = glm::rotateY(Direction, -dYawRadians);
      UpVector = glm::rotateY(UpVector, dYawRadians);

      glm::vec3 right = glm::cross(Direction, UpVector);
      Direction = glm::rotate(Direction, dPitchRadians, right);
      UpVector = glm::rotate(UpVector, dPitchRadians, right);
   }
}
