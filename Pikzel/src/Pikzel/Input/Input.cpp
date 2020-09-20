#include "Input.h"

#include "Pikzel/Events/EventDispatcher.h"
#include "Pikzel/Events/KeyEvents.h"

#include <GLFW/glfw3.h>

namespace Pikzel {

   namespace Input {

      enum class InputKeyState {
         Up,
         Down
      };


      std::unordered_map<entt::id_type, float> g_Axes = {
         {"Horizontal"_hs, 0.0f},
         {"Vertical"_hs, 0.0f}
      };


      std::unordered_map<int, std::pair<entt::id_type, float>> g_MappedKeys = {
         {GLFW_KEY_W,     {"Vertical"_hs,    1.0f}},
         {GLFW_KEY_A,     {"Horizontal"_hs, -1.0f}},
         {GLFW_KEY_S,     {"Vertical"_hs,   -1.0f}},
         {GLFW_KEY_D,     {"Horizontal"_hs,  1.0f}},
         {GLFW_KEY_UP,    {"Vertical"_hs,    1.0f}},
         {GLFW_KEY_LEFT,  {"Horizontal"_hs, -1.0f}},
         {GLFW_KEY_RIGHT, {"Horizontal"_hs,  1.0f}},
         {GLFW_KEY_DOWN,  {"Vertical"_hs,   -1.0f}},
      };


      std::unordered_map<int, InputKeyState> g_State;


      void OnKeyPressed(const KeyPressedEvent& event) {
         for (auto [keyCode, axis] : g_MappedKeys) {
            if (event.KeyCode == keyCode) {
               if (g_State[keyCode] == InputKeyState::Down) {
                  return;
               }
               auto [id, delta] = axis;
               g_Axes[id] += delta; // Floating point issues here?
               g_State[keyCode] = InputKeyState::Down; 
            }
         }
      }


      void OnKeyReleased(const KeyReleasedEvent& event) {
         for (auto [keyCode, axis] : g_MappedKeys) {
            if (event.KeyCode == keyCode) {
               if (g_State[keyCode] == InputKeyState::Up) {
                  // Should not be possible...
                  PKZL_CORE_ASSERT(FALSE, "KeyReleasedEvent when key is not down!");
                  return;
               }
               auto [id, delta] = axis;
               g_Axes[id] -= delta; // Floating point issues here?
               g_State[keyCode] = InputKeyState::Up;
            }
         }
      }


      // TOOD: Later, there could be parameters to Init that let the client set their own Axes and mappings to them
      void Init() {
         EventDispatcher::Connect<KeyPressedEvent, OnKeyPressed>();
         EventDispatcher::Connect<KeyReleasedEvent, OnKeyReleased>();
      }


      float GetAxis(entt::id_type id) {
         return g_Axes.at(id);
      }

   }
}
