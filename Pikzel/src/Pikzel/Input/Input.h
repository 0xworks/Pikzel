#pragma once

#include "KeyCodes.h"
#include "MouseButtons.h"

#include "Pikzel/Core/Core.h"
#include "Pikzel/Core/Window.h"
#include "Pikzel/Events/ApplicationEvents.h"
#include "Pikzel/Events/KeyEvents.h"
#include "Pikzel/Events/MouseEvents.h"

#include <unordered_map>

struct GLFWwindow;

namespace Pikzel {

   using DeltaTime = std::chrono::duration<float, std::chrono::seconds::period>;

   class PKZL_API Input {
   public:

      // later we might have some other things in here... such as choice of input "scheme" to use...
      struct Settings {
         float MouseSensitivity = 0.01f;
      };

      Input(const Window& window, const Input::Settings& settings = {});

      float GetAxis(entt::id_type id) const;

      bool IsKeyPressed(KeyCode key) const;

      bool IsMouseButtonPressed(MouseButton button) const;

   private:
      void OnUpdate(const UpdateEvent& event);
      void OnKeyPressed(const KeyPressedEvent& event);
      void OnKeyReleased(const KeyReleasedEvent& event);
      void OnMouseScrolled(const MouseScrolledEvent& event);

   private:

      Settings m_Settings;

      enum class KeyState {
         Up,
         Down
      };

      // The intent here is that (one day) clients will be able to create their own axes (and map keys to them).
      // For instance a "pitch" or "yaw" axis...
      std::unordered_map<entt::id_type, float> m_Axes;
      std::unordered_map<KeyCode, std::pair<entt::id_type, float>> m_MappedKeys;

      std::unordered_map<KeyCode, KeyState> m_KeyState;

      GLFWwindow* m_Window; // HACK! (TODO: make better.  e.g. have an abstract base "Input" and this class is "WindowsInput" or something)

      double m_MouseX = 0.0f;
      double m_MouseY = 0.0f;
      float m_MouseDeltaZ = 0.0f;
      float m_MouseDeltaW = 0.0f;

   };
}
