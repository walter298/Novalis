#include "LayerDropdown.h"

#include "WindowLayout.h"

void nv::editor::LayerDropdown::showLayerCreationWindow(NodeEditor& currTab) {
	m_showingAddNewLayerPopup = true;

	centerNextWindow();

	ImGui::OpenPopup(LAYER_CREATION_POPUP_NAME);
	ImGui::BeginPopupContextWindow(LAYER_CREATION_POPUP_NAME);

	centerNextText("Create Layer");
	ImGui::Text("Create Layer");

	ImGui::InputText("Layer Name", &m_layerName);

	centerNextText("Create");
	if (ImGui::Button("Create")) {
		currTab.addLayer(m_layerName);
		m_showingAddNewLayerPopup = false;
		ImGui::CloseCurrentPopup();
	}

	ImGui::EndPopup();
}

void nv::editor::LayerDropdown::show(Project& project) {
	if (!project.getCurrentTab() || project.getCurrentTab()->isBusy()) {
		showDisabledMenu("Layer");
		return;
	}

	if (m_showingAddNewLayerPopup) {
		showLayerCreationWindow(*project.getCurrentTab());
	}

	if (ImGui::BeginMenu("Layer")) {
		if (ImGui::MenuItem("New Layer")) {
			m_showingAddNewLayerPopup = true;
		}
		if (ImGui::MenuItem("Delete Layer")) {

		}
		if (ImGui::MenuItem("Duplicate Layer")) {

		}
		ImGui::EndMenu();
	}
}
