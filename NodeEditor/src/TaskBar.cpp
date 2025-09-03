#include "ErrorPopup.h"
#include "FileDropdown.h"
#include "ProjectManager.h"
#include "TaskBar.h"

void nv::editor::TaskBar::show(SDL_Renderer* renderer, ProjectManager& projectManager, ErrorPopup& errorPopup) {
	ImGui::BeginMainMenuBar();
	
	showFileDropdown(projectManager, errorPopup);

	auto currProject = projectManager.getCurrentProject();
	if (currProject) {
		m_layerCreator.show(*currProject);
		showObjectDropdown(renderer, *currProject, errorPopup);
	} else {
		showDisabledMenu("Layer");
		showDisabledMenu("Object");
	}
	
	ImGui::EndMainMenuBar();
}

bool nv::editor::TaskBar::isBusy() const noexcept {
	return isObjectDropdownBusy();
}
