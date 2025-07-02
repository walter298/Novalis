#pragma once

#include <string>

#include "NodeTabList.h"

namespace nv {
	namespace editor {
		class LayerDropdown {
		private:
			std::string m_layerName;
			bool m_showingAddNewLayerPopup = false;

			void showLayerCreationWindow(NodeEditor& currTab);
		public:
			void show(NodeTabList& tabs) {
				if (!tabs.currentTab() || tabs.currentTab()->isBusy()) {
					showDisabledMenu("Layer");
					return;
				}

				if (m_showingAddNewLayerPopup) {
					showLayerCreationWindow(*tabs.currentTab());
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
		};
	}
}