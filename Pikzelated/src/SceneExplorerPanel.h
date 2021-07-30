#pragma once

#include "Panel.h"

#include <string>

class SceneExplorerPanel final : public Panel {
public:
	SceneExplorerPanel(SceneEditor& editor);
	~SceneExplorerPanel() = default;

public:
	virtual void Render() override;

private:
	std::string m_Name;
};
