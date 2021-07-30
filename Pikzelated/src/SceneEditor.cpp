#include "SceneEditor.h"

// TODO: get these from somewhere
const float nearPlane = 1000.1f;
const float farPlane = 0.1f;


SceneEditor::SceneEditor(std::unique_ptr<Pikzel::Scene> scene)
: m_Scene{std::move(scene)}
{
   m_Camera.projection = glm::perspective(m_Camera.fovRadians, static_cast<float>(m_ViewportSize.x) / static_cast<float>(m_ViewportSize.y), nearPlane, farPlane);
}


Pikzel::Scene& SceneEditor::GetScene() {
   PKZL_ASSERT(m_Scene, "Attempted to access null scene!");
   return *m_Scene;
}


void SceneEditor::SetScene(std::unique_ptr<Pikzel::Scene> scene) {
   m_Scene = std::move(scene);
}


Camera& SceneEditor::GetCamera() {
   return m_Camera;
}


glm::u32vec2 SceneEditor::GetViewportSize() const {
   return m_ViewportSize;
}


void SceneEditor::SetViewportSize(const glm::u32vec2 size) {
   m_ViewportSize = size;
   m_Camera.projection = glm::perspective(m_Camera.fovRadians, static_cast<float>(m_ViewportSize.x) / static_cast<float>(m_ViewportSize.y), nearPlane, farPlane);
}


void SceneEditor::Update(const Pikzel::Input& input, const Pikzel::DeltaTime deltaTime) {
   m_Camera.Update(input, deltaTime);
}
