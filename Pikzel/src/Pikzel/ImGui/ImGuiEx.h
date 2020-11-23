#pragma once

#include "Pikzel/Core/Window.h"

#include <glm/glm.hpp>
#include <imgui.h>

namespace Pikzel {

   namespace ImGuiEx {

      void Init(Window& window);

      void EditVec3(const char* label, glm::vec3& value, const float resetValue = 0.0f, const float labelWidth = 100.0f);
      void EditVec3Color(const char* label, glm::vec3& value, const float labelWidth = 100.0f);

   }
}
