#include "Input.h"

#include "Pikzel/Core/Window.h"
#include "Pikzel/Events/EventDispatcher.h"

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

namespace Pikzel {

   using DeltaTime = std::chrono::duration<float, std::chrono::seconds::period>;

   Input::Input(const Window& window, const Settings& settings)
   : m_Window {static_cast<GLFWwindow*>(window.GetNativeWindow())}
   , m_Settings(settings)
   , m_Axes {
      {"X"_hs, 0.0f},
      {"Y"_hs, 0.0f},
      {"Z"_hs, 0.0f},
      {"MouseX"_hs, 0.0f},
      {"MouseY"_hs, 0.0f},
      {"MouseZ"_hs, 0.0f}
   }
   , m_MappedKeys {
      {KeyCode::D,        {"X"_hs,  1.0f}},
      {KeyCode::A,        {"X"_hs, -1.0f}},
      {KeyCode::R,        {"Y"_hs,  1.0f}},
      {KeyCode::F,        {"Y"_hs, -1.0f}},
      {KeyCode::W,        {"Z"_hs,  1.0f}},
      {KeyCode::S,        {"Z"_hs, -1.0f}},
      {KeyCode::Right,    {"X"_hs,  1.0f}},
      {KeyCode::Left,     {"X"_hs, -1.0f}},
      {KeyCode::PageUp,   {"Y"_hs,  1.0f}},
      {KeyCode::PageDown, {"Y"_hs, -1.0f}},
      {KeyCode::Up,       {"Z"_hs,  1.0f}},
      {KeyCode::Down,     {"Z"_hs, -1.0f}},
   }
   {
      EventDispatcher::Connect<UpdateEvent, &Input::OnUpdate>(*this);
      EventDispatcher::Connect<KeyPressedEvent, &Input::OnKeyPressed>(*this);
      EventDispatcher::Connect<KeyReleasedEvent, &Input::OnKeyReleased>(*this);
      EventDispatcher::Connect<MouseScrolledEvent, &Input::OnMouseScrolled>(*this);
      glfwGetCursorPos(m_Window, &m_MouseX, &m_MouseY);
   }


   float Input::GetAxis(entt::id_type id) const {
      return m_Axes.at(id);
   }


   bool Input::IsKeyPressed(KeyCode key) const {
      auto state = glfwGetKey(m_Window, static_cast<int32_t>(key)); 
      return state == GLFW_PRESS || state == GLFW_REPEAT;
   }


   bool Input::IsMouseButtonPressed(MouseButton button) const {
      auto state = glfwGetMouseButton(m_Window, static_cast<int>(button));
      return state == GLFW_PRESS;
   }


   void Input::OnUpdate(const UpdateEvent& event) {
      if (event.deltaTime.count() > 0.0f) {
         double x;
         double y;
         glfwGetCursorPos(m_Window, &x, &y);
         m_Axes["MouseX"_hs] = static_cast<float>(x -  m_MouseX) * m_Settings.MouseSensitivity / event.deltaTime.count();
         m_Axes["MouseY"_hs] = static_cast<float>(m_MouseY - y) * m_Settings.MouseSensitivity / event.deltaTime.count(); // nb: cursor Y axis is inverted relative to world Y axis
         m_Axes["MouseZ"_hs] = m_MouseDeltaZ / event.deltaTime.count();
         m_MouseX = x;
         m_MouseY = y;
         m_MouseDeltaZ = 0.0f;
         m_MouseDeltaW = 0.0f;
      }
   }


   void Input::OnKeyPressed(const KeyPressedEvent& event) {
      for (auto [keyCode, axis] : m_MappedKeys) {
         if (event.KeyCode == keyCode) {
            if (m_KeyState[keyCode] == KeyState::Up) {
               auto [id, delta] = axis;
               m_Axes[id] += delta; // Floating point issues here?
               m_KeyState[keyCode] = KeyState::Down;
            }
            break;
         }
      }
   }


   void Input::OnKeyReleased(const KeyReleasedEvent& event) {
      for (auto [keyCode, axis] : m_MappedKeys) {
         if (event.KeyCode == keyCode) {
            if (m_KeyState[keyCode] == KeyState::Down) {
               auto [id, delta] = axis;
               m_Axes[id] -= delta; // Floating point issues here?
               m_KeyState[keyCode] = KeyState::Up;
            }
            break;
         }
      }
   }


   void Input::OnMouseScrolled(const MouseScrolledEvent& event) {
      if (event.Sender == m_Window) {
         m_MouseDeltaZ = event.YOffset;
         m_MouseDeltaW = event.XOffset;
      }
   }
}
