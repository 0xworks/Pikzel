#include "PropertiesPanel.h"
#include "UI.h"

#include "Pikzel/Components/Model.h"
#include "Pikzel/Components/Transform.h"

#include "Pikzel/Events/EventDispatcher.h"

#include "Pikzel/ImGui/ImGuiEx.h" // TODO: get rid of this

#include "Pikzel/Scene/AssetCache.h"

using namespace Pikzel;

static ImGuiWindowFlags s_PropertiesPanelFlags = ImGuiWindowFlags_NoCollapse;

PropertiesPanel::PropertiesPanel(SceneEditor& editor)
: Panel{editor}
{
   m_Name = ICON_FA_CUBES" Properties###Properties";
   EventDispatcher::Connect<SceneEditor::ObjectSelectedEvent, &PropertiesPanel::OnObjectSelected>(*this);
}


template<typename Component, typename Callback> requires std::is_invocable_v<Callback, Component&>
void DrawComponent(std::string_view componentName, Scene& scene, Object object, Callback callback) {
   if(auto component = scene.TryGetComponent<Component>(object)) {
      ImGuiTreeNodeFlags extraFlags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_FramePadding;
      auto [expanded, clicked] = UI::IconTreeNode((void*)typeid(Component).hash_code(), UI::Icon::Component, componentName, extraFlags);
      if (expanded) {
         if (callback) {
            callback(*component);
         }
         ImGui::TreePop();
      }
   }
}


void PropertiesPanel::Render() {
   static bool showPropertiesPanel = true;
   showPropertiesPanel = IsVisible();
   auto& scene = GetEditor().GetScene();
   ImGui::Begin(m_Name.data(), &showPropertiesPanel, s_PropertiesPanelFlags);
   {
      if (m_SelectedObject != Null) {
         auto& name = scene.GetComponent<std::string>(m_SelectedObject);
         char buffer[256];
         strncpy(buffer, name.c_str(), 256);
         ImGui::Text(ICON_FA_PENCIL);
         ImGui::SameLine();
         if (ImGui::InputText("##Name", buffer, 256)) {
            name = buffer;
         }

         DrawComponent<Transform>("Transform", scene, m_SelectedObject, [](Transform& transform) {
            UI::BeginPropertyTable("Transform");
            bool bChanged = UI::Property("Translation", transform.Translation);
            bChanged |= UI::Property("Rotation", transform.RotationEuler);
            bChanged |= UI::Property("Scale", transform.Scale, glm::vec3{1.0f});
            UI::EndPropertyTable();
            if (bChanged) {
               // compute rotation quat
               // re-compute matrix
            }
         });
         
         DrawComponent<Model>("Model", scene, m_SelectedObject, [](Model& model) {
            UI::BeginPropertyTable("Model");

            // need to think about this a bit more...
            // basically we need to be able to drag n drop assets into this property.
            // we also want a drop down selectable menu of assets...
            if (model.id != Null) {
               // and if user changes it.. its not that we want the model's name changed
               // what we mean is to change to a different model
               auto temp = AssetCache::GetPath(model.id)->string();
               UI::Property("Model", temp);
            }
            UI::EndPropertyTable();
         });
      }
   }
   ImGui::End();

   // Carry out action from ImGui popup menus _after_ finishing ImGui rendering
   // (because the actions themselves can affect what ImGui needs to render)
   if(m_Action) {
      m_Action();
      m_Action = {};
   }

   SetVisible(showPropertiesPanel);
}


void PropertiesPanel::OnObjectSelected(const SceneEditor::ObjectSelectedEvent& event) {
   m_SelectedObject = event.object;
}
