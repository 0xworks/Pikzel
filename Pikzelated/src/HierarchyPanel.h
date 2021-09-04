#pragma once

#include "Panel.h"

#include <imgui.h>

#include <functional>
#include <string>
#include <unordered_set>

// Hierarchy 
class HierarchyPanel final : public Panel {
public:
   HierarchyPanel(SceneEditor& editor);
   ~HierarchyPanel() = default;

public:
   virtual void Render() override;

private:
   void RenderObject(Pikzel::Scene& scene, Pikzel::Object object, Pikzel::Object childObject);
   void RenderAddMenu(Pikzel::Object object);
   void RenderDeleteMenu(Pikzel::Object object);

private:
   // m_Action provides a means to defer executing a function until the end of the Render() loop, i.e. when we are ready for it.
   std::function<void()> m_Action;
   std::unordered_set<Pikzel::Object> m_EnsureExpanded;
   std::string m_Name;
   Pikzel::Object m_SelectedObject;
};
