#include "UI.h"

#include "Pikzel/ImGui/ImGuiEx.h"
#include "Pikzel/Renderer/RenderCore.h"
#include "Pikzel/Renderer/sRGB.h"

#include <fonts/FontAwesome6Regular400.inl>
#include <fonts/FontAwesome6Solid900.inl>
#include <fonts/RobotoBold.inl>
#include <fonts/RobotoRegular.inl>

#include <imgui_internal.h>

namespace UI {

   const char* IconToGlyph(Icon icon) {
      switch (icon) {
         case Icon::Scene:     return ICON_FA_GLOBE;
         case Icon::Object:    return ICON_FA_CUBES;
         case Icon::Component: return ICON_FA_CUBE;
      }
      return ICON_FA_QUESTION;
   }


   ImVec4 IconToColor(Icon icon) {
      // TODO: theme aware colors.  For now choose them carefully so they work on either dark or light background
      switch (icon) {
         case Icon::Scene:  return {0.59f, 0.42f, 0.00f, 1.0f};
         case Icon::Object: return {0.32f, 0.87f, 0.32f, 1.0f};
         case Icon::Component: return {0.32f, 0.70f, 0.87f, 1.0f};
      }
      return {0.0f, 0.0f, 0.0f, 1.0f};
   }


   std::tuple<bool, bool> IconTreeNode(void* ptr_id, Icon icon, std::string_view content, ImGuiTreeNodeFlags extraFlags, std::function<void()> callback) {
      bool isExpanded = ImGui::TreeNodeEx(ptr_id, ImGuiTreeNodeFlags_SpanFullWidth | extraFlags, "");
      bool isClicked = ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByPopup) && (ImGui::IsMouseClicked(ImGuiMouseButton_Left) || ImGui::IsMouseClicked(ImGuiMouseButton_Right));
      if (callback) {
         callback();
      }
      ImGui::PushStyleColor(ImGuiCol_Text, IconToColor(icon));
      ImGui::SameLine();
      ImGui::Text(IconToGlyph(icon));
      ImGui::PopStyleColor();
      ImGui::SameLine();
      ImGui::Text(content.data());
      return {isExpanded, isClicked};
   }


   void BeginPropertyTable(std::string_view id) {
      ImGui::BeginTable(id.data(), 2, ImGuiTableFlags_Resizable);
   }


   bool Property(std::string_view label, glm::vec3& value, const glm::vec3& resetValue) {
      ImGui::TableNextRow();
      ImGui::TableSetColumnIndex(0);
      ImGui::Text(label.data());
      ImGui::TableSetColumnIndex(1);
      return Pikzel::ImGuiEx::EditVec3(&value, resetValue);
   }


   bool Property(std::string_view label, std::string& value) {
      ImGui::TableNextRow();
      ImGui::TableSetColumnIndex(0);
      ImGui::Text(label.data());
      ImGui::TableSetColumnIndex(1);
      char buffer[256];
      strncpy(buffer, value.c_str(), 256);
      return ImGui::InputText("##value", buffer, 256);
   }

   void EndPropertyTable() {
      ImGui::EndTable();
   }

}
