#pragma once

#include "Panel.h"

#include <imgui.h>
#include <string>

// Hierarchy 
class HierarchyPanel final : public Panel {
public:
   HierarchyPanel(SceneEditor& editor);
   ~HierarchyPanel() = default;

public:
   virtual void Render() override;

private:
   void RenderObject(const Pikzel::Scene& scene, Pikzel::Object object, Pikzel::Object childObject);
   void RenderAddMenu(Pikzel::Object object);
   void RenderDeleteMenu(Pikzel::Object object);

private:
   std::function<void()> m_Action;
   std::string m_Name;
   Pikzel::Object m_SelectedObject;
};
