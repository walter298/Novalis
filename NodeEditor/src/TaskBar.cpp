#include "TaskBar.h"

void nv::editor::TaskBar::show(SDL_Renderer* renderer, ProjectManager& projectManager, ErrorPopup& errorPopup) {
	ImGui::BeginMainMenuBar();
	m_fileDropdown.show(projectManager, errorPopup);

	auto currProject = projectManager.getCurrentProject();
	if (currProject) {
		m_layerCreator.show(*currProject);
		m_objectLoader.show(renderer, *currProject, errorPopup);
	} else {
		showDisabledMenu("Layer");
		showDisabledMenu("Object");
	}
	
	ImGui::EndMainMenuBar();
}

bool nv::editor::TaskBar::isBusy() const noexcept {
	return m_objectLoader.isBusy();
}
