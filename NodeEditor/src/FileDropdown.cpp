#include <novalis/detail/file/File.h>

#include "FileDropdown.h"
#include "WindowLayout.h"

//for static methods
using namespace nv;
using namespace editor;

static void openProject(plf::hive<Project>& projects) {
	auto file = openFile({ { "project files", "json" } });
	if (file) {
		
	}
}

static void saveProject(Project& project, ErrorPopup& errorPopup) {
	std::ofstream file{ project.getRootDirectory() };
	if (!file.is_open()) {
		errorPopup.add("Could not open project file: " + project.getRootDirectory().string());
	} else {
		file << nlohmann::json{ project }.dump(2);
	}
}

void nv::editor::FileDropdown::show(ProjectManager& projectManager, ErrorPopup& errorPopup) {
	if (m_creatingProject) {
		if (m_projectCreator.create(projectManager, errorPopup)) {
			m_creatingProject = false;
		}
	}

	if (ImGui::BeginMenu("File")) {
		if (ImGui::MenuItem("New Project")) {
			m_creatingProject = true;
		}
		if (ImGui::MenuItem("Open Project")) {
			//tabs.upload();
		}
		ImGui::Separator();
		ImGui::BeginDisabled(!projectManager.getCurrentProject());
		if (ImGui::MenuItem("Save Project")) {
			saveProject(*projectManager.getCurrentProject(), errorPopup);
		}
		ImGui::EndDisabled();

		ImGui::EndMenu();
	}
}