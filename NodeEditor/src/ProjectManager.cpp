#include <novalis/detail/file/File.h>
#include <novalis/detail/ScopeExit.h>

#include "ProjectManager.h"
#include "WindowLayout.h"

void nv::editor::ProjectManager::tryLoadProject(ErrorPopup& errorPopup) {
	try {
		auto projectPath = openFile({ { "Projects", "json" } });
		if (!projectPath) {
			return;
		}
		std::ifstream file{ *projectPath };
		auto projectJson = nlohmann::json::parse(file);
		auto project = projectJson.get<Project>();
		m_currProject = &(*m_projects.insert(std::move(project)));
	} catch (const nlohmann::json::exception& e) {
		errorPopup.add(std::format("Error loading project: {}", e.what()));
	}
}

bool nv::editor::ProjectManager::createProject(bool& cancelled, ErrorPopup& errorPopup) {
	if (m_projectCreator.create(cancelled, errorPopup)) {
		const auto& projectName = m_projectCreator.getProjectNameInput();
		if (m_projectNames.contains(projectName)) {
			errorPopup.add(std::format("Error: another project named {} is currently loaded", projectName));
			return false;
		}

		auto oldIDCount = FileID::IDCount;
		auto oldDirIDCount = DirectoryID::IDCount;
		FileID::IDCount = 0;
		DirectoryID::IDCount = 0;

		try {
			m_currProject = &(*m_projects.emplace(m_projectCreator.getCurrentDirectory(), projectName));
			m_projectNames.insert(projectName);
			return true;
		} catch (const std::filesystem::filesystem_error& e) {
			FileID::IDCount = oldIDCount;
			DirectoryID::IDCount = oldDirIDCount;
			errorPopup.add(std::format("Error creating project : {}", e.what()));
		}
	}
	return false;
}

bool nv::editor::ProjectManager::switchProject(bool& cancelled, ErrorPopup& errorPopup) noexcept {
	ImGui::SetNextWindowSize({ 800.0f, 800.0f });
	centerNextWindow();
	ImGui::OpenPopup(PROJECT_LOAD_POPUP_NAME);
	if (ImGui::BeginPopup(PROJECT_LOAD_POPUP_NAME, DEFAULT_WINDOW_FLAGS)) {
		nv::detail::ScopeExit endPopup{ []() { ImGui::EndPopup(); } }; //popup cleanup
		for (auto& project : m_projects) {
			ImGui::SetNextItemWidth(getInputWidth());
			if (ImGui::Button(project.getName().c_str())) {
				//EXTREMELY IMPORTANT: update the ID counts!!
				if (!m_currProject->save(errorPopup)) {
					cancelled = true;
					return false;
				}
				m_currProject = &project;
				FileID::IDCount = project.getFileIDOffset();
				DirectoryID::IDCount = project.getDirectoryIDOffset();
				return true;
			}
		}
		ImGui::SetNextItemWidth(getInputWidth());
		if (ImGui::Button("Cancel")) {
			cancelled = true;
		}
	}
	return false;
}

boost::optional<nv::editor::Project&> nv::editor::ProjectManager::getCurrentProject() {
	if (m_currProject) {
		return *m_currProject;
	} else {
		return boost::none;
	}
}