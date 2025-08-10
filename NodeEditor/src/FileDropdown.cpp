#include <novalis/detail/file/File.h>

#include "ErrorPopup.h"
#include "FileDropdown.h"
#include "ProjectCreator.h"
#include "ProjectManager.h"
#include "WindowLayout.h"

//for static methods
using namespace nv;
using namespace editor;

static void saveProject(Project& project, ErrorPopup& errorPopup) {
	std::ofstream file{ project.getRootDirectory() / "project.json" };
	if (!file.is_open()) {
		errorPopup.add("Could not open project file: " + project.getRootDirectory().string());
	} else {
		file << nlohmann::json{ project }.dump(2);
	}
}

void nv::editor::FileDropdown::show(ProjectManager& projectManager, ErrorPopup& errorPopup) {
	switch (m_state) {
	case SwitchingProject:
		if (projectManager.switchProject()) {
			m_state = None;
		}
		break;
	case CreatingProject:
		bool cancelled = false;
		if (projectManager.createProject(cancelled, errorPopup) || cancelled) {
			m_state = None;
		}
		break;
	}

	if (ImGui::BeginMenu("File")) {
		if (ImGui::MenuItem("New Project")) {
			m_state = CreatingProject;
		}
		if (ImGui::MenuItem("Open Project")) {
			projectManager.tryLoadProject(errorPopup);
		}
		if (ImGui::MenuItem("Switch Project")) {
			m_state = SwitchingProject;
		}

		auto currProject = projectManager.getCurrentProject();
		ImGui::Separator();
		ImGui::BeginDisabled(!currProject.has_value());
		if (ImGui::MenuItem("Save Project")) {
			currProject->save(errorPopup);
		}
		ImGui::EndDisabled();
		ImGui::EndMenu();
	}
}