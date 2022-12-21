#pragma once

#include "Panel.h"
#include "SceneEditor.h"

#include "Pikzel/Core/Core.h"
#include "Pikzel/Scene/Scene.h"

#include <functional>
#include <string>

class PropertiesPanel final : public Panel {
public:
   PropertiesPanel(SceneEditor& editor);
   ~PropertiesPanel() = default;

public:
   virtual void Render() override;

private:
   void OnObjectSelected(const SceneEditor::ObjectSelectedEvent& event);

private:
   // m_Action provides a means to defer executing a function until the end of the Render() loop, i.e. when we are ready for it.
   std::function<void()> m_Action;
   std::string m_Name;
   Pikzel::Object m_SelectedObject = Pikzel::Null;
};
