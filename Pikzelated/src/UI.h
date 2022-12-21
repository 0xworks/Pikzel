#pragma once

#include "Pikzel/Core/Window.h"
#include "Pikzel/ImGui/IconsFontAwesome6.h"

#include <imgui.h>

#include <string_view>

namespace UI {

   enum class Icon {
      Scene,
      Object,
      Component
   };

   std::tuple<bool, bool> IconTreeNode(void* ptr_id, Icon icon, std::string_view content, ImGuiTreeNodeFlags extraFlags = ImGuiTreeNodeFlags_None, std::function<void()> callback = {});

   void BeginPropertyTable(std::string_view id);
   bool Property(std::string_view label, glm::vec3& value, const glm::vec3& resetValue = {0.0f, 0.0f, 0.0f});
   bool Property(std::string_view label, std::string& value);
   void EndPropertyTable();

}
