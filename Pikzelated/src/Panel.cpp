#include "Panel.h"

Panel::Panel(SceneEditor& editor)
: m_Editor{editor}
{}


SceneEditor& Panel::GetEditor() {
	return m_Editor;
}


bool Panel::IsVisible() const {
	return m_IsVisible;
}


void Panel::SetVisible(const bool b) {
	m_IsVisible = b;
}
