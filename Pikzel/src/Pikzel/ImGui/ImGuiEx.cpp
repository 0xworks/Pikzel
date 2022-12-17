#include "ImGuiEx.h"

#include "Pikzel/Renderer/RenderCore.h"
#include "Pikzel/Renderer/sRGB.h"

#include <fonts/CousineRegular.inl>
#include <fonts/CousineBold.inl>
#include <fonts/FontAwesome6Regular400.inl>
#include <fonts/FontAwesome6Solid900.inl>

#include <imgui_internal.h>

namespace Pikzel {
   namespace ImGuiEx {

      void AddFont(const void* compressedData, const int compressedSize, const float pixelSize) {
         static const ImWchar icon_ranges[] = {ICON_MIN_FA, ICON_MAX_FA, 0};

         auto& io = ImGui::GetIO();
         io.Fonts->AddFontFromMemoryCompressedTTF(compressedData, compressedSize, pixelSize);

         ImFontConfig cfg;
         cfg.GlyphMinAdvanceX = pixelSize;
         cfg.MergeMode = true;

         ImFont* mergedFont = io.Fonts->AddFontFromMemoryCompressedTTF(FontAwesome6Regular400_compressed_data, FontAwesome6Regular400_compressed_size, pixelSize * 0.75f, &cfg, icon_ranges);
         PKZL_CORE_ASSERT(mergedFont, "Failed to merge font 'FontAwesome6Regular400'!");
         mergedFont = io.Fonts->AddFontFromMemoryCompressedTTF(FontAwesome6Solid900_compressed_data, FontAwesome6Solid900_compressed_size, pixelSize * 0.75f, &cfg, icon_ranges);
         PKZL_CORE_ASSERT(mergedFont, "Failed to merge font 'FontAwesome6Solid900'!");
      }


      void Init(Window& window) {
         window.InitializeImGui();
         ImGui::SetCurrentContext(window.GetGraphicsContext().GetImGuiContext());

         ImGuiIO& io = ImGui::GetIO();
         io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
         io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
         io.ConfigWindowsMoveFromTitleBarOnly = true;

         SetColors(Theme::Dark);

         float scaleFactor = window.ContentScale();
         ImGuiStyle& style = ImGui::GetStyle();
         style.ScaleAllSizes(scaleFactor);
         style.WindowRounding = 0.0f;
         style.TabRounding = 0.0f;

         AddFont(CousineRegular_compressed_data, CousineRegular_compressed_size, 16 * scaleFactor);
         AddFont(CousineBold_compressed_data, CousineBold_compressed_size, 16 * scaleFactor);

         RenderCore::UploadImGuiFonts();
      }


      void SetColors(Theme theme) {
         ImVec4* colors = ImGui::GetStyle().Colors;
         switch (theme) {
            case Theme::Light:
               ImGui::StyleColorsLight();
               colors[ImGuiCol_Text]                   = sRGBA{0.00f, 0.00f, 0.00f, 1.00f};
               colors[ImGuiCol_TextDisabled]           = sRGBA{0.60f, 0.60f, 0.60f, 1.00f};
               colors[ImGuiCol_WindowBg]               = sRGBA{0.94f, 0.94f, 0.94f, 0.90f};
               colors[ImGuiCol_ChildBg]                = sRGBA{0.94f, 0.94f, 0.94f, 1.00f};
               colors[ImGuiCol_PopupBg]                = sRGBA{1.00f, 1.00f, 1.00f, 1.00f};
               colors[ImGuiCol_Border]                 = sRGBA{0.00f, 0.00f, 0.00f, 1.00f};
               colors[ImGuiCol_BorderShadow]           = sRGBA{0.00f, 0.00f, 0.00f, 1.00f};
               colors[ImGuiCol_FrameBg]                = sRGBA{1.00f, 1.00f, 1.00f, 1.00f};
               colors[ImGuiCol_FrameBgHovered]         = sRGBA{0.26f, 0.59f, 0.98f, 1.00f};
               colors[ImGuiCol_FrameBgActive]          = sRGBA{0.26f, 0.59f, 0.98f, 1.00f};
               colors[ImGuiCol_TitleBg]                = sRGBA{0.96f, 0.96f, 0.96f, 1.00f};
               colors[ImGuiCol_TitleBgActive]          = sRGBA{0.82f, 0.82f, 0.82f, 1.00f};
               colors[ImGuiCol_TitleBgCollapsed]       = sRGBA{1.00f, 1.00f, 1.00f, 1.00f};
               colors[ImGuiCol_MenuBarBg]              = sRGBA{0.86f, 0.86f, 0.86f, 1.00f};
               colors[ImGuiCol_ScrollbarBg]            = sRGBA{0.98f, 0.98f, 0.98f, 1.00f};
               colors[ImGuiCol_ScrollbarGrab]          = sRGBA{0.69f, 0.69f, 0.69f, 1.00f};
               colors[ImGuiCol_ScrollbarGrabHovered]   = sRGBA{0.49f, 0.49f, 0.49f, 1.00f};
               colors[ImGuiCol_ScrollbarGrabActive]    = sRGBA{0.49f, 0.49f, 0.49f, 1.00f};
               colors[ImGuiCol_CheckMark]              = sRGBA{0.26f, 0.59f, 0.98f, 1.00f};
               colors[ImGuiCol_SliderGrab]             = sRGBA{0.26f, 0.59f, 0.98f, 1.00f};
               colors[ImGuiCol_SliderGrabActive]       = sRGBA{0.46f, 0.54f, 0.80f, 1.00f};
               colors[ImGuiCol_Button]                 = sRGBA{0.26f, 0.59f, 0.98f, 1.00f};
               colors[ImGuiCol_ButtonHovered]          = sRGBA{0.26f, 0.59f, 0.98f, 1.00f};
               colors[ImGuiCol_ButtonActive]           = sRGBA{0.06f, 0.53f, 0.98f, 1.00f};
               colors[ImGuiCol_Header]                 = sRGBA{0.26f, 0.59f, 0.98f, 1.00f};
               colors[ImGuiCol_HeaderHovered]          = sRGBA{0.26f, 0.59f, 0.98f, 0.60f};
               colors[ImGuiCol_HeaderActive]           = sRGBA{0.26f, 0.59f, 0.98f, 1.00f};
               colors[ImGuiCol_Separator]              = sRGBA{0.39f, 0.39f, 0.39f, 1.00f};
               colors[ImGuiCol_SeparatorHovered]       = sRGBA{0.14f, 0.44f, 0.80f, 1.00f};
               colors[ImGuiCol_SeparatorActive]        = sRGBA{0.14f, 0.44f, 0.80f, 1.00f};
               colors[ImGuiCol_ResizeGrip]             = sRGBA{0.35f, 0.35f, 0.35f, 1.00f};
               colors[ImGuiCol_ResizeGripHovered]      = sRGBA{0.26f, 0.59f, 0.98f, 1.00f};
               colors[ImGuiCol_ResizeGripActive]       = sRGBA{0.26f, 0.59f, 0.98f, 1.00f};
               colors[ImGuiCol_Tab]                    = sRGBA{0.76f, 0.80f, 0.84f, 1.00f};
               colors[ImGuiCol_TabHovered]             = sRGBA{0.26f, 0.59f, 0.98f, 1.00f};
               colors[ImGuiCol_TabActive]              = sRGBA{0.60f, 0.73f, 0.88f, 1.00f};
               colors[ImGuiCol_TabUnfocused]           = sRGBA{0.92f, 0.93f, 0.94f, 1.00f};
               colors[ImGuiCol_TabUnfocusedActive]     = sRGBA{0.74f, 0.82f, 0.91f, 1.00f};
               colors[ImGuiCol_DockingPreview]         = sRGBA{0.26f, 0.59f, 0.98f, 1.00f};
               colors[ImGuiCol_DockingEmptyBg]         = sRGBA{0.20f, 0.20f, 0.20f, 1.00f};
               colors[ImGuiCol_PlotLines]              = sRGBA{0.39f, 0.39f, 0.39f, 1.00f};
               colors[ImGuiCol_PlotLinesHovered]       = sRGBA{1.00f, 0.43f, 0.35f, 1.00f};
               colors[ImGuiCol_PlotHistogram]          = sRGBA{0.90f, 0.70f, 0.00f, 1.00f};
               colors[ImGuiCol_PlotHistogramHovered]   = sRGBA{1.00f, 0.45f, 0.00f, 1.00f};
               colors[ImGuiCol_TableHeaderBg]          = sRGBA{0.78f, 0.87f, 0.98f, 1.00f};
               colors[ImGuiCol_TableBorderStrong]      = sRGBA{0.57f, 0.57f, 0.64f, 1.00f};
               colors[ImGuiCol_TableBorderLight]       = sRGBA{0.68f, 0.68f, 0.74f, 1.00f};
               colors[ImGuiCol_TableRowBg]             = sRGBA{0.00f, 0.00f, 0.00f, 1.00f};
               colors[ImGuiCol_TableRowBgAlt]          = sRGBA{0.30f, 0.30f, 0.30f, 1.00f};
               colors[ImGuiCol_TextSelectedBg]         = sRGBA{0.26f, 0.59f, 0.98f, 1.00f};
               colors[ImGuiCol_DragDropTarget]         = sRGBA{0.26f, 0.59f, 0.98f, 1.00f};
               colors[ImGuiCol_NavHighlight]           = sRGBA{0.26f, 0.59f, 0.98f, 1.00f};
               colors[ImGuiCol_NavWindowingHighlight]  = sRGBA{0.70f, 0.70f, 0.70f, 1.00f};
               colors[ImGuiCol_NavWindowingDimBg]      = sRGBA{0.20f, 0.20f, 0.20f, 1.00f};
               colors[ImGuiCol_ModalWindowDimBg]       = sRGBA{0.20f, 0.20f, 0.20f, 1.00f};
               break;

            case Theme::Dark:
               ImGui::StyleColorsDark();
               colors[ImGuiCol_Text]                   = sRGBA{1.00f, 1.00f, 1.00f, 1.00f};
               colors[ImGuiCol_TextDisabled]           = sRGBA{0.50f, 0.50f, 0.50f, 1.00f};
               colors[ImGuiCol_WindowBg]               = sRGBA{0.06f, 0.06f, 0.06f, 0.94f};
               colors[ImGuiCol_ChildBg]                = sRGBA{0.00f, 0.00f, 0.00f, 1.00f};
               colors[ImGuiCol_PopupBg]                = sRGBA{0.08f, 0.08f, 0.08f, 1.00f};
               colors[ImGuiCol_Border]                 = sRGBA{0.43f, 0.43f, 0.50f, 1.00f};
               colors[ImGuiCol_BorderShadow]           = sRGBA{0.00f, 0.00f, 0.00f, 1.00f};
               colors[ImGuiCol_FrameBg]                = sRGBA{0.16f, 0.29f, 0.48f, 1.00f};
               colors[ImGuiCol_FrameBgHovered]         = sRGBA{0.26f, 0.59f, 0.98f, 1.00f};
               colors[ImGuiCol_FrameBgActive]          = sRGBA{0.26f, 0.59f, 0.98f, 1.00f};
               colors[ImGuiCol_TitleBg]                = sRGBA{0.04f, 0.04f, 0.04f, 1.00f};
               colors[ImGuiCol_TitleBgActive]          = sRGBA{0.16f, 0.29f, 0.48f, 1.00f};
               colors[ImGuiCol_TitleBgCollapsed]       = sRGBA{0.00f, 0.00f, 0.00f, 1.00f};
               colors[ImGuiCol_MenuBarBg]              = sRGBA{0.14f, 0.14f, 0.14f, 1.00f};
               colors[ImGuiCol_ScrollbarBg]            = sRGBA{0.02f, 0.02f, 0.02f, 1.00f};
               colors[ImGuiCol_ScrollbarGrab]          = sRGBA{0.31f, 0.31f, 0.31f, 1.00f};
               colors[ImGuiCol_ScrollbarGrabHovered]   = sRGBA{0.41f, 0.41f, 0.41f, 1.00f};
               colors[ImGuiCol_ScrollbarGrabActive]    = sRGBA{0.51f, 0.51f, 0.51f, 1.00f};
               colors[ImGuiCol_CheckMark]              = sRGBA{0.26f, 0.59f, 0.98f, 1.00f};
               colors[ImGuiCol_SliderGrab]             = sRGBA{0.24f, 0.52f, 0.88f, 1.00f};
               colors[ImGuiCol_SliderGrabActive]       = sRGBA{0.26f, 0.59f, 0.98f, 1.00f};
               colors[ImGuiCol_Button]                 = sRGBA{0.26f, 0.59f, 0.98f, 1.00f};
               colors[ImGuiCol_ButtonHovered]          = sRGBA{0.26f, 0.59f, 0.98f, 1.00f};
               colors[ImGuiCol_ButtonActive]           = sRGBA{0.06f, 0.53f, 0.98f, 1.00f};
               colors[ImGuiCol_Header]                 = sRGBA{0.26f, 0.59f, 0.98f, 1.00f};
               colors[ImGuiCol_HeaderHovered]          = sRGBA{0.26f, 0.59f, 0.98f, 0.60f};
               colors[ImGuiCol_HeaderActive]           = sRGBA{0.26f, 0.59f, 0.98f, 1.00f};
               colors[ImGuiCol_Separator]              = sRGBA{0.43f, 0.43f, 0.50f, 1.00f};
               colors[ImGuiCol_SeparatorHovered]       = sRGBA{0.10f, 0.40f, 0.75f, 1.00f};
               colors[ImGuiCol_SeparatorActive]        = sRGBA{0.10f, 0.40f, 0.75f, 1.00f};
               colors[ImGuiCol_ResizeGrip]             = sRGBA{0.26f, 0.59f, 0.98f, 1.00f};
               colors[ImGuiCol_ResizeGripHovered]      = sRGBA{0.26f, 0.59f, 0.98f, 1.00f};
               colors[ImGuiCol_ResizeGripActive]       = sRGBA{0.26f, 0.59f, 0.98f, 1.00f};
               colors[ImGuiCol_Tab]                    = sRGBA{0.18f, 0.35f, 0.58f, 1.00f};
               colors[ImGuiCol_TabHovered]             = sRGBA{0.26f, 0.59f, 0.98f, 1.00f};
               colors[ImGuiCol_TabActive]              = sRGBA{0.20f, 0.41f, 0.68f, 1.00f};
               colors[ImGuiCol_TabUnfocused]           = sRGBA{0.07f, 0.10f, 0.15f, 1.00f};
               colors[ImGuiCol_TabUnfocusedActive]     = sRGBA{0.14f, 0.26f, 0.42f, 1.00f};
               colors[ImGuiCol_DockingPreview]         = sRGBA{0.26f, 0.59f, 0.98f, 1.00f};
               colors[ImGuiCol_DockingEmptyBg]         = sRGBA{0.20f, 0.20f, 0.20f, 1.00f};
               colors[ImGuiCol_PlotLines]              = sRGBA{0.61f, 0.61f, 0.61f, 1.00f};
               colors[ImGuiCol_PlotLinesHovered]       = sRGBA{1.00f, 0.43f, 0.35f, 1.00f};
               colors[ImGuiCol_PlotHistogram]          = sRGBA{0.90f, 0.70f, 0.00f, 1.00f};
               colors[ImGuiCol_PlotHistogramHovered]   = sRGBA{1.00f, 0.60f, 0.00f, 1.00f};
               colors[ImGuiCol_TableHeaderBg]          = sRGBA{0.19f, 0.19f, 0.20f, 1.00f};
               colors[ImGuiCol_TableBorderStrong]      = sRGBA{0.31f, 0.31f, 0.35f, 1.00f};
               colors[ImGuiCol_TableBorderLight]       = sRGBA{0.23f, 0.23f, 0.25f, 1.00f};
               colors[ImGuiCol_TableRowBg]             = sRGBA{0.00f, 0.00f, 0.00f, 1.00f};
               colors[ImGuiCol_TableRowBgAlt]          = sRGBA{1.00f, 1.00f, 1.00f, 1.00f};
               colors[ImGuiCol_TextSelectedBg]         = sRGBA{0.26f, 0.59f, 0.98f, 1.00f};
               colors[ImGuiCol_DragDropTarget]         = sRGBA{1.00f, 1.00f, 0.00f, 1.00f};
               colors[ImGuiCol_NavHighlight]           = sRGBA{0.26f, 0.59f, 0.98f, 1.00f};
               colors[ImGuiCol_NavWindowingHighlight]  = sRGBA{1.00f, 1.00f, 1.00f, 1.00f};
               colors[ImGuiCol_NavWindowingDimBg]      = sRGBA{0.80f, 0.80f, 0.80f, 1.00f};
               colors[ImGuiCol_ModalWindowDimBg]       = sRGBA{0.80f, 0.80f, 0.80f, 1.00f};
               break;
         }
      }


      const char* IconToString(Icon icon) {
         switch (icon) {
            case Icon::Scene:  return ICON_FA_GLOBE;
            case Icon::Object: return ICON_FA_CUBE;
         }
         return ICON_FA_QUESTION;
      }


      ImVec4 IconToColor(Icon icon) {
         // TODO: theme aware colors.  For now choose them carefully so they work on either dark or light background
         switch (icon) {
            case Icon::Scene:  return {0.59f, 0.42f, 0.00f, 1.0f};
            case Icon::Object: return {0.32f, 0.70f, 0.87f, 1.0f};
         }
         return {0.0f, 0.0f, 0.0f, 1.0f};
      }

      std::tuple<bool, bool> IconTreeNode(void* ptr_id, Icon icon, std::string_view content, ImGuiTreeNodeFlags extraFlags, std::function<void()> callback) {
         bool isExpanded = ImGui::TreeNodeEx(ptr_id, ImGuiTreeNodeFlags_SpanFullWidth | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_OpenOnArrow | extraFlags, "");
         bool isClicked = ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByPopup) && (ImGui::IsMouseClicked(ImGuiMouseButton_Left) || ImGui::IsMouseClicked(ImGuiMouseButton_Right));
         if (callback) {
            callback();
         }
         ImGui::PushStyleColor(ImGuiCol_Text, IconToColor(icon));
         ImGui::SameLine();
         ImGui::Text(IconToString(icon));
         ImGui::PopStyleColor();
         ImGui::SameLine();
         ImGui::Text(content.data());
         return {isExpanded, isClicked};
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

         auto boldFont = ImGui::GetIO().Fonts->Fonts[(int)Font::UIBold];

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

         ImGui::DragFloat("##Float", value, 1.0f, 0.0f, 100000.0f, format, ImGuiSliderFlags_Logarithmic);

         ImGui::PopStyleVar();
         ImGui::Columns(1);
         ImGui::PopID();
      }

   }
}
