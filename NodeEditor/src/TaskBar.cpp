#include "TaskBar.h"

void nv::editor::TaskBar::show(SDL_Renderer* renderer, NodeTabList& tabs) {
	ImGui::BeginMainMenuBar();
	m_fileDropdown.show(tabs);
	m_layerCreator.show(tabs);
	m_objectLoader.show(renderer, tabs);
	ImGui::EndMainMenuBar();
}

bool nv::editor::TaskBar::isBusy() const noexcept {
	return m_objectLoader.creatingSpritesheet();
}
