#include <filesystem>
#include <novalis/detail/file/File.h>

#include "imgui/imgui.h" //IMGUI_API
#include "imgui/imgui_stdlib.h"

#include "ProjectCreator.h"
#include "WindowLayout.h"

bool nv::editor::ProjectCreator::create(bool& cancelled, ErrorPopup& errorPopup) {
	bool created = false;

	ImGui::OpenPopup(PROJECT_CREATION_POPUP_NAME);

	if (ImGui::BeginPopup(PROJECT_CREATION_POPUP_NAME)) {
		ImGui::SetNextItemWidth(getInputWidth());
		ImGui::InputText("Project Name", &m_projectNameInput);

		ImGui::SetNextItemWidth(getInputWidth());
		auto directoryStr = m_directoryLocation.string();
		if (ImGui::Button(directoryStr.c_str())) {
			auto projectLocationRes = nv::openDirectory();
			if (projectLocationRes) {
				m_directoryLocation = std::move(*projectLocationRes);
			}
		}

		ImGui::SetNextItemWidth(getInputWidth());
		if (ImGui::Button("Create Project")) {
			if (m_projectNameInput.empty()) {
				errorPopup.add("Error: project name can't be empty");
			} else if (!std::filesystem::exists(m_directoryLocation)) {
				errorPopup.add(std::format("Error: {} does not exist", directoryStr));
			} else {
				created = true;
			}
		}

		ImGui::SetNextItemWidth(getInputWidth());
		if (ImGui::Button("Cancel")) {
			cancelled = true;
		}
		ImGui::EndPopup();
	}
	return created;
}

const std::filesystem::path& nv::editor::ProjectCreator::getCurrentDirectory() {
	return m_directoryLocation;
}

const std::string& nv::editor::ProjectCreator::getProjectNameInput() {
	return m_projectNameInput;
}
