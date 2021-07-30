#pragma once

#include "SceneEditor.h"

class Panel {
public:
	Panel(SceneEditor& editor);
	virtual ~Panel() = default;

public:
	SceneEditor& GetEditor();

	bool IsVisible() const;
	void SetVisible(const bool b);

	virtual void Render() = 0;

private:
	SceneEditor& m_Editor;
	bool m_IsVisible = true;
};
