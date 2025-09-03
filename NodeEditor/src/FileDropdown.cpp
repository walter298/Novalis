#include <novalis/detail/file/File.h>

#include "ErrorPopup.h"
#include "FileDropdown.h"
#include "ProjectCreator.h"
#include "ProjectManager.h"
#include "WindowLayout.h"

//for static methods
using namespace nv;
using namespace editor;

namespace {
	enum FileDropdownState {
		None,
		CreatingProject,
		SwitchingProject,
		SwitchingProjectVersion
	};
	FileDropdownState state = None;

	void executeState(ProjectManager& projectManager, ErrorPopup& errorPopup) {
		bool cancelled = false;
		switch (state) {
		case SwitchingProject:
			if (projectManager.switchProject(cancelled, errorPopup) || cancelled) {
				state = None;
			}
			break;
		case CreatingProject:
			if (projectManager.createProject(cancelled, errorPopup) || cancelled) {
				state = None;
			}
			break;
		case SwitchingProjectVersion:
			if (projectManager.getCurrentProject()->switchVersion(cancelled, errorPopup) || cancelled) {
				state = None;
			} 
			break;
		}
	}
	void showDropdown(ProjectManager& projectManager, ErrorPopup& errorPopup) {
		if (ImGui::BeginMenu("File")) {
			if (ImGui::MenuItem("New Project")) {
				state = CreatingProject;
			}

			ImGui::Separator();
			if (ImGui::MenuItem("Open Project")) {
				projectManager.tryLoadProject(errorPopup);
			}
			if (ImGui::MenuItem("Switch Project")) {
				state = SwitchingProject;
			}
			
			auto currProject = projectManager.getCurrentProject();
			if (currProject) {
				ImGui::Separator();
				if (ImGui::MenuItem("Save Project")) {
					currProject->save(errorPopup);
				}
				if (ImGui::MenuItem("Switch Project Version")) {
					state = SwitchingProjectVersion;
				}
			}
			ImGui::EndMenu();
		}
	}
}

void nv::editor::showFileDropdown(ProjectManager& projectManager, ErrorPopup& errorPopup) {
	showDropdown(projectManager, errorPopup);
	executeState(projectManager, errorPopup);
}
