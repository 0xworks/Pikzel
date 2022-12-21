#include "HierarchyPanel.h"
#include "UI.h"

#include "Pikzel/Components/Relationship.h"
#include "Pikzel/Events/EventDispatcher.h"

using namespace Pikzel;

static ImGuiWindowFlags s_HierarchyFlags = ImGuiWindowFlags_NoCollapse;

HierarchyPanel::HierarchyPanel(SceneEditor& editor)
: Panel{editor}
{
   m_Name = ICON_FA_BARS_STAGGERED" Hierarchy###Hierarchy";
   EventDispatcher::Connect<SceneEditor::ObjectSelectedEvent, &HierarchyPanel::OnObjectSelected>(*this);
}


void HierarchyPanel::Render() {
   static bool showHierarchy = true;
   showHierarchy = IsVisible();
   auto& scene = GetEditor().GetScene();
   ImGui::Begin(m_Name.data(), &showHierarchy, s_HierarchyFlags);
   {
      auto [expanded, clicked] = UI::IconTreeNode(0, UI::Icon::Scene, "Scene", ImGuiTreeNodeFlags_DefaultOpen, [this, &scene] {

         if (ImGui::BeginPopupContextItem()) {
            RenderAddMenu(Null);
            ImGui::EndPopup();
         }

         bool isDropTargetValid = true;
         const ImGuiPayload* payload = ImGui::GetDragDropPayload();
         if (payload) {
            if (payload->IsDataType("Object")) {
               Object sourceObject = *static_cast<Object*>(payload->Data);

               // Scene is not a valid drop target if the source already has no parent.
               if (scene.GetComponent<Relationship>(sourceObject).Parent == Null) {
                  isDropTargetValid = false;
               }
            }
            // could validate other types of payload here
         }

         if (isDropTargetValid && ImGui::BeginDragDropTarget()) {
            const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("Object");
            if (payload) {
               Object sourceObject = *static_cast<Object*>(payload->Data);
               m_Action = [&scene, sourceObject] {
                  scene.GetComponent<Relationship>(sourceObject).Parent = Null;
                  scene.SortObjects();
               };
            }
            ImGui::EndDragDropTarget();
         }
      });

      if (clicked) {
         Object object = Null;
         EventDispatcher::Send<SceneEditor::ObjectSelectedEvent>(object);
      }

      if (expanded) {
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

   SetVisible(showHierarchy);
}


void HierarchyPanel::RenderObject(Scene& scene, Object object, Object childObject) {
   ImGuiTreeNodeFlags extraFlags = ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_OpenOnArrow;
   if (object == m_SelectedObject) {
      extraFlags |= ImGuiTreeNodeFlags_Selected;
   }
   if (childObject == Null) {
      extraFlags |= ImGuiTreeNodeFlags_Leaf;
   }
   if (m_EnsureExpanded.contains(object)) {
      ImGui::SetNextItemOpen(true);
      m_EnsureExpanded.erase(object);
   }
   auto& name = scene.GetComponent<std::string>(object);
   auto [expanded, clicked] = UI::IconTreeNode((void*)(uintptr_t)object, UI::Icon::Object, name, extraFlags, [this, &scene, object] {

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

      bool isDropTargetValid = true;
      const ImGuiPayload* payload = ImGui::GetDragDropPayload();
      if (payload) {
         if (payload->IsDataType("Object")) {
            Object sourceObject = *static_cast<Object*>(payload->Data);

            // Drop target is only valid if object is not already the parent of source.
            if (scene.GetComponent<Relationship>(sourceObject).Parent == object) {
               isDropTargetValid = false;
            } else {
               // Drop target is only valid if source is not an ancestor of object.
               // In other words, you cannot drag an object onto one of its descendants (that would make it a descendant of its descendant - circular and confusing)
               Object parent = object;
               while (parent != Null) {
                  if (parent == sourceObject) {
                     isDropTargetValid = false;
                     break;
                  }
                  parent = scene.GetComponent<Relationship>(parent).Parent;
               }
            }
         }
         // could validate other types of payload here
      }

      if (isDropTargetValid && ImGui::BeginDragDropTarget()) {
         const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("Object");
         if (payload) {
            Object sourceObject = *static_cast<Object*>(payload->Data);
            m_Action = [this, &scene, sourceObject, object] {
               scene.GetComponent<Relationship>(sourceObject).Parent = object;
               scene.SortObjects();
               m_EnsureExpanded.insert(object);
            };
         }
         ImGui::EndDragDropTarget();
      }

   });
   if (clicked) {
      EventDispatcher::Send<SceneEditor::ObjectSelectedEvent>(object);
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
         auto& scene = GetEditor().GetScene();
         Object parent = scene.GetComponent<Relationship>(object).Parent;
         scene.DestroyObject(object);
         EventDispatcher::Send<SceneEditor::ObjectSelectedEvent>(parent);
      };
   }
}


void HierarchyPanel::OnObjectSelected(const SceneEditor::ObjectSelectedEvent& event) {
   m_SelectedObject = event.object;
}
