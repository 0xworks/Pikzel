#include "SceneExplorerPanel.h"

#include "Pikzel/ImGui/ImGuiEx.h"


static ImGuiWindowFlags s_SceneExplorerFlags = ImGuiWindowFlags_NoCollapse;

SceneExplorerPanel::SceneExplorerPanel(SceneEditor& editor)
   : Panel{editor}
{
   m_Name = ICON_FA_GLOBE "  Scene Explorer###SceneExplorer";
}


void SceneExplorerPanel::Render() {
   static bool showSceneExplorer = true;
   showSceneExplorer = IsVisible();
   ImGui::Begin(m_Name.data(), &showSceneExplorer, s_SceneExplorerFlags);
   {
      for (auto [entity, id] : GetEditor().GetScene().GetView<Pikzel::Id>().each()) {
         ImGui::Text(std::format("{}", id).data());
      }
   }
   ImGui::End();
   SetVisible(showSceneExplorer);
}
