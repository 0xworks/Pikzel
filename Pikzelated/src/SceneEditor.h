#pragma once

#include "Pikzel/Scene/Camera.h"
#include "Pikzel/Scene/Scene.h"

#include <glm/glm.hpp>

#include <memory>

class SceneEditor final {
public:

   SceneEditor(std::unique_ptr<Pikzel::Scene> scene);
   ~SceneEditor() = default;

   Pikzel::Scene& GetScene();
   void SetScene(std::unique_ptr<Pikzel::Scene> scene);

   Camera& GetCamera();

   glm::u32vec2 GetViewportSize() const;
   void SetViewportSize(const glm::u32vec2 size);

   void Update(const Pikzel::Input& input, const Pikzel::DeltaTime deltaTime);

private:
   Camera m_Camera = {
      .position = {0.0f, 0.0f, 3.0f},
      .direction = glm::normalize(glm::vec3{0.0f, 0.0f, -1.0f}),
      .upVector = {0.0f, 1.0f, 0.0f},
      .fovRadians = glm::radians(60.f),
      .moveSpeed = 2.5f,
      .rotateSpeed = 10.0f
   };

   glm::u32vec2 m_ViewportSize = {800, 600};

   std::unique_ptr<Pikzel::Scene> m_Scene;
};
