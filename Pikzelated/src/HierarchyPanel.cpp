#include "HierarchyPanel.h"

#include "Pikzel/Components/Relationship.h"
#include "Pikzel/ImGui/ImGuiEx.h"

using namespace Pikzel;

static ImGuiWindowFlags s_SceneExplorerFlags = ImGuiWindowFlags_NoCollapse;

HierarchyPanel::HierarchyPanel(SceneEditor& editor)
   : Panel{editor}
{
   m_Name = ICON_FA_STREAM"  Hierarchy###Hierarchy";
}


void HierarchyPanel::Render() {
   static bool showSceneExplorer = true;
   showSceneExplorer = IsVisible();
   ImGui::Begin(m_Name.data(), &showSceneExplorer, s_SceneExplorerFlags);
   {
      auto [expanded, clicked] = ImGuiEx::IconTreeNode(0, ImGuiEx::Icon::Scene, "Scene", ImGuiTreeNodeFlags_DefaultOpen);
      if (clicked) {
         m_SelectedObject = Null;
      }
      if (expanded) {
         auto& scene = GetEditor().GetScene();
         Object object = scene.GetView<Relationship>().front();
         while (object != Null) {
            auto& relationship = scene.GetComponent<Relationship>(object);
            RenderObject(scene, object, relationship.FirstChild);
            object = relationship.NextSibling;
         }
         ImGui::TreePop();
      }
   }
   if (ImGui::BeginPopupContextWindow(nullptr, ImGuiPopupFlags_MouseButtonDefault_ | ImGuiPopupFlags_NoOpenOverItems)) {
      RenderAddMenu(Null);
      ImGui::EndPopup();
   }
   ImGui::End();

   // Carry out action from ImGui popup menus _after_ finishing ImGui rendering
   // (because the actions themselves can affect what ImGui needs to render)
   if(m_Action) {
      m_Action();
      m_Action = {};
   }

   SetVisible(showSceneExplorer);
}


void HierarchyPanel::RenderObject(Scene& scene, Object object, Object childObject) {
   ImGuiTreeNodeFlags extraFlags = ImGuiTreeNodeFlags_None;
   if (object == m_SelectedObject) {
      extraFlags |= ImGuiTreeNodeFlags_Selected;
   }
   if (childObject == Null) {
      extraFlags |= ImGuiTreeNodeFlags_Leaf;
   }
   auto& name = scene.GetComponent<std::string>(object);
   auto [expanded, clicked] = ImGuiEx::IconTreeNode((void*)(uintptr_t)object, ImGuiEx::Icon::Object, name.data(), extraFlags, [this, &scene, object] {

      if (ImGui::BeginPopupContextItem()) {
         RenderAddMenu(object);
         RenderDeleteMenu(object);
         ImGui::EndPopup();
      }

      if (ImGui::BeginDragDropSource()) {
         ImGui::Text(scene.GetComponent<std::string>(object).data());
         ImGui::SetDragDropPayload("Object", &object, sizeof(Object));
         ImGui::EndDragDropSource();
      }

      // Begin target only if source is not one of this objects parents.
      // In other words, you cannot drag an object onto one of it's children (that would make it a child of its child.. circular and confusing)
      bool isDescendent = false;
      const ImGuiPayload* payload = ImGui::GetDragDropPayload();
      if (payload && payload->IsDataType("Object")) {
         Object sourceObject = *static_cast<Object*>(payload->Data);
         Object parent = object;
         while (parent != Null) {
            if (parent == sourceObject) {
               isDescendent = true;
               break;
            }
            parent = scene.GetComponent<Relationship>(parent).Parent;
         }
      }

      if (!isDescendent && ImGui::BeginDragDropTarget()) {
         const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("Object");
         if (payload) {
            Object sourceObject = *static_cast<Object*>(payload->Data);
            m_Action = [&scene, sourceObject, object] {
               scene.GetComponent<Relationship>(sourceObject).Parent = object;
               scene.SortObjects();
            };
         }
         ImGui::EndDragDropTarget();
      }

   });
   if (clicked) {
      PKZL_CORE_LOG_INFO("Clicked");
      m_SelectedObject = object;
   }
   if(expanded) {
      while (childObject != Null) {
         auto childRelationship = scene.GetComponent<Relationship>(childObject);
         RenderObject(scene, childObject, childRelationship.FirstChild);
         childObject = childRelationship.NextSibling;
      }
      ImGui::TreePop();
   }
}


void HierarchyPanel::RenderAddMenu(Object parent) {
   if (ImGui::BeginMenu("Add")) {
      if (ImGui::MenuItem("Empty Object")) {
         m_Action = [this, parent] {
            GetEditor().GetScene().CreateObject(parent);
         };
      }
      ImGui::EndMenu();
   }
}


void HierarchyPanel::RenderDeleteMenu(Object object) {
   if (ImGui::MenuItem("Delete")) {
      m_Action = [this, object] {
         GetEditor().GetScene().DestroyObject(object);
      };
   }
}
