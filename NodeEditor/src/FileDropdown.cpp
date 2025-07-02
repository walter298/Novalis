#include "FileDropdown.h"

#include "WindowLayout.h"

void nv::editor::FileDropdown::showNodeCreationPopupWindow(NodeTabList& tabs)
{
	auto center = ImGui::GetMainViewport()->GetCenter();
	ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

	ImGui::OpenPopup(NODE_CREATION_POPUP_NAME);
	ImGui::BeginPopupContextWindow(NODE_CREATION_POPUP_NAME);

	centerNextText("Create Node");
	ImGui::Text("Create Node");

	ImGui::InputText("Node Name", &m_nodeNameInput);

	centerNextText("Create");
	if (ImGui::Button("Create")) {
		m_showingNodeCreationPopupWindow = false;
		tabs.add(m_nodeNameInput);
		ImGui::CloseCurrentPopup();
	}
	ImGui::EndPopup();
}

void nv::editor::FileDropdown::show(NodeTabList& tabs) {
	if (m_showingNodeCreationPopupWindow) {
		showNodeCreationPopupWindow(tabs);
	}

	if (ImGui::BeginMenu("File")) {
		if (ImGui::MenuItem("New Node")) {
			m_showingNodeCreationPopupWindow = true;
		}
		if (ImGui::MenuItem("Open Node")) {
			tabs.upload();
		}
		ImGui::Separator();
		ImGui::BeginDisabled(!tabs.currentTab().has_value());
		if (ImGui::MenuItem("Save")) {
			tabs.currentTab()->save();
		}
		if (ImGui::MenuItem("Save as")) {
			tabs.currentTab()->saveAs();
		}
		ImGui::EndDisabled();
		ImGui::EndMenu();
	}
}