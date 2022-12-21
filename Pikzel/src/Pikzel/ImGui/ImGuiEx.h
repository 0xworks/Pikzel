#pragma once

#include "Pikzel/Core/Window.h"
#include "Pikzel/ImGui/IconsFontAwesome6.h"

#include <glm/ext/vector_float3.hpp>

namespace Pikzel::ImGuiEx {

   enum class Theme {
      Light,
      Dark
   };

   enum class Font {
      RobotoRegular,
      RobotoBold
   };

   void Init(Window& window);
   void SetColors(Theme theme);

   bool EditVec3(glm::vec3* value, const glm::vec3& resetValue = {0.0f, 0.0f, 0.0f});
   bool EditVec3(const char* label, glm::vec3* value, const glm::vec3& resetValue = {0.0f, 0.0f, 0.0f}, const float labelWidth = 100.0f);

   bool EditVec3Color(const char* label, glm::vec3* value, const float labelWidth = 100.0f);
   bool EditFloat(const char* label, float* value, const float labelWidth = 100.0f, const char* format = "%.3f", ImGuiInputTextFlags flags = 0);

}
