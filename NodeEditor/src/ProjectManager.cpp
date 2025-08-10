#include <novalis/detail/file/File.h>

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
		try {
			m_currProject = &(*m_projects.emplace(m_projectCreator.getCurrentDirectory(), projectName));
			m_projectNames.insert(projectName);
			return true;
		} catch (const std::filesystem::filesystem_error& e) {
			errorPopup.add(std::format("Error creating project : {}", e.what()));
		}
	}
	return false;
}

bool nv::editor::ProjectManager::switchProject() noexcept {
	ImGui::SetNextWindowSize({ 800.0f, 800.0f });
	centerNextWindow();
	ImGui::OpenPopup(PROJECT_LOAD_POPUP_NAME);
	if (ImGui::BeginPopup(PROJECT_LOAD_POPUP_NAME, DEFAULT_WINDOW_FLAGS)) {
		for (auto& project : m_projects) {
			ImGui::SetNextItemWidth(getInputWidth());
			if (ImGui::Button(project.name.c_str())) {
				m_currProject = &project;
				ImGui::EndPopup();
				return true;
			}
		}
		ImGui::EndPopup();
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