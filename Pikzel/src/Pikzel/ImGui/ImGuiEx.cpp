#include "ImGuiEx.h"

#include "Pikzel/Renderer/RenderCore.h"

#include <imgui/imgui_internal.h>

namespace Pikzel {
   namespace ImGuiEx {

      void Init(Window& window) {
         window.InitializeImGui();

         // Pikzel default ImGui style
         ImGui::SetCurrentContext(window.GetGraphicsContext().GetImGuiContext());
         ImGuiIO& io = ImGui::GetIO();
         ImGui::StyleColorsDark();
         ImGuiStyle& style = ImGui::GetStyle();
         if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
            style.WindowRounding = 0.0f;
            style.Colors[ImGuiCol_WindowBg].w = 1.0f;
         }
         float scaleFactor = window.ContentScale();
         style.ScaleAllSizes(scaleFactor);

         io.Fonts->AddFontFromFileTTF("Assets/Fonts/OpenSans-Bold.ttf", 16 * scaleFactor);
         io.FontDefault = io.Fonts->AddFontFromFileTTF("Assets/Fonts/OpenSans-Regular.ttf", 16 * scaleFactor);

         RenderCore::UploadImGuiFonts();
      }


      void EditVec3(const char* label, glm::vec3* value, const float resetValue, const float labelWidth) {
         //
         // ImGui UI for glm::vec3 based on code from TheCherno Game Engine Series episode 91  https://www.youtube.com/watch?v=IEiOP7Y-Mbc&list=PLlrATfBNZ98dC-V-N3m0Go4deliWHPFwT&index=91
         //
         ImGui::PushID(label);

         ImGui::Columns(2, nullptr, false);
         ImGui::SetColumnWidth(0, labelWidth);
         ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2 {10, 10});

         ImGui::AlignTextToFramePadding();
         ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ImGui::GetColumnWidth() - ImGui::CalcTextSize(label).x - ImGui::GetScrollX() - 2 * ImGui::GetStyle().ItemSpacing.x);
         ImGui::Text(label);

         ImGui::NextColumn();
         ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth());

         ImVec4 button = ImGui::GetStyleColorVec4(ImGuiCol_Button);
         ImGui::PushStyleColor(ImGuiCol_ButtonHovered, button);
         ImGui::PushStyleColor(ImGuiCol_ButtonActive, button);

         auto boldFont = ImGui::GetIO().Fonts->Fonts[0];

         {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4 {0.8f, 0.1f, 0.15f, 1.0f});
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4 {0.9f, 0.2f, 0.2f, 1.0f});
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4 {0.8f, 0.1f, 0.15f, 1.0f});
            ImGui::PushFont(boldFont);
            if (ImGui::Button("X")) {
               value->x = resetValue;
            }
            ImGui::PopFont();
            ImGui::PopStyleColor(3);

            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2 {0, 0});
            ImGui::SameLine();
            ImGui::DragFloat("##X", &value->x, 0.1f, 0.0f, 0.0f, "%.2f");
            ImGui::PopStyleVar();
         }
         ImGui::PopItemWidth();

         {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4 {0.2f, 0.7f, 0.2f, 1.0f});
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4 {0.3f, 0.8f, 0.3f, 1.0f});
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4 {0.2f, 0.7f, 0.2f, 1.0f});
            ImGui::SameLine();
            ImGui::PushFont(boldFont);
            if (ImGui::Button("Y")) {
               value->y = resetValue;
            }
            ImGui::PopFont();
            ImGui::PopStyleColor(3);

            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2 {0, 0});
            ImGui::SameLine();
            ImGui::DragFloat("##Y", &value->y, 0.1f, 0.0f, 0.0f, "%.2f");
            ImGui::PopStyleVar();
         }
         ImGui::PopItemWidth();

         {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4 {0.1f, 0.25f, 0.8f, 1.0f});
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4 {0.2f, 0.35f, 0.9f, 1.0f});
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4 {0.1f, 0.25f, 0.8f, 1.0f});
            ImGui::SameLine();
            ImGui::PushFont(boldFont);
            if (ImGui::Button("Z")) {
               value->z = resetValue;
            }
            ImGui::PopFont();
            ImGui::PopStyleColor(3);

            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2 {0, 0});
            ImGui::SameLine();
            ImGui::DragFloat("##Z", &value->z, 0.1f, 0.0f, 0.0f, "%.2f");
            ImGui::PopStyleVar();
         }
         ImGui::PopItemWidth();

         ImGui::PopStyleVar();
         ImGui::PopStyleColor(2);
         ImGui::Columns(1);
         ImGui::PopID();
      }


      void EditVec3Color(const char* label, glm::vec3* value, const float labelWidth) {
         ImGui::PushID(label);

         ImGui::Columns(2, nullptr, false);
         ImGui::SetColumnWidth(0, labelWidth);
         ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2 {10, 10});

         ImGui::AlignTextToFramePadding();
         ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ImGui::GetColumnWidth() - ImGui::CalcTextSize(label).x - ImGui::GetScrollX() - 2 * ImGui::GetStyle().ItemSpacing.x);
         ImGui::Text(label);

         ImGui::NextColumn();

         float color[3] = {value->r, value->g, value->b};
         ImGui::ColorEdit3("##Color", color, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_NoBorder);
         *value = {color[0], color[1], color[2]};

         ImGui::PopStyleVar();
         ImGui::Columns(1);
         ImGui::PopID();
      }

      void EditFloat(const char* label, float* value, const float labelWidth, const char* format, ImGuiInputTextFlags flags) {
         ImGui::PushID(label);

         ImGui::Columns(2, nullptr, false);
         ImGui::SetColumnWidth(0, labelWidth);
         ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2 {10, 10});

         ImGui::AlignTextToFramePadding();
         ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ImGui::GetColumnWidth() - ImGui::CalcTextSize(label).x - ImGui::GetScrollX() - 2 * ImGui::GetStyle().ItemSpacing.x);
         ImGui::Text(label);

         ImGui::NextColumn();

         ImGui::InputFloat("##Float", value, 0.0, 0.0, format, flags);

         ImGui::PopStyleVar();
         ImGui::Columns(1);
         ImGui::PopID();
      }

   }
}
