#pragma once

#include "Pikzel/Core/Window.h"
#include "Pikzel/ImGui/IconsFontAwesome5.h"

#include <glm/glm.hpp>

namespace Pikzel {

   namespace ImGuiEx {

      enum class Theme {
         Light,
         Dark
      };

      enum class Font {
         UIRegular,
         UIBold
      };

      enum class Icon {
         Scene,
         Object
      };

      void Init(Window& window);
      void SetColors(Theme theme);

      std::tuple<bool, bool> IconTreeNode(void* ptr_id, Icon icon, std::string_view content, ImGuiTreeNodeFlags extraFlags = ImGuiTreeNodeFlags_None, std::function<void()> callback = {});

      void EditVec3(const char* label, glm::vec3* value, const float resetValue = 0.0f, const float labelWidth = 100.0f);
      void EditVec3Color(const char* label, glm::vec3* value, const float labelWidth = 100.0f);
      void EditFloat(const char* label, float* value, const float labelWidth = 100.0f, const char* format = "%.3f", ImGuiInputTextFlags flags = 0);
   }
}
