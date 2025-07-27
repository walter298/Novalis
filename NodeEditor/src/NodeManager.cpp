#include "NodeManager.h"
#include "VirtualFilesystem.h"

namespace {
	bool loaded = false;
}
void nv::editor::NodeManager::addNode(FileID id) {
	auto it = m_nodes.emplace(id, id);
	m_currTab = &it.first->second;
	loaded = true;
}

void nv::editor::NodeManager::removeNode(FileID id) {
	if (m_currTab && m_currTab->getID() == id) {
		m_currTab = nullptr;
	}
	m_nodes.erase(id);
}

void nv::editor::NodeManager::showTabs(VirtualFilesystem& fs) {
	auto tabWindowPos = getTabWindowPos();
	auto tabWindowSize = getTabWindowSize();
	ImGui::SetNextWindowPos(tabWindowPos);
	ImGui::SetNextWindowSize(tabWindowSize);

	ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4{ 0.0f, 0.0f, 0.0f, 0.f });

	ImGui::Begin(TAB_WINDOW_NAME, nullptr, DEFAULT_WINDOW_FLAGS);

	bool deletedTab = false;

	if (ImGui::BeginTabBar("Tabs")) {
		for (auto& [id, tab] : m_nodes) {
			ImGui::PushID(getTemporaryImGuiID());
			
			if (ImGui::BeginTabItem(fs.getFilename(id).c_str())) {
				m_currTab = &tab;
				ImGui::Text(tab.getName());
				ImGui::EndTabItem();
			}
			ImGui::PopID();
		}
		ImGui::EndTabBar();
	}

	ImGui::PopStyleColor();

	ImGui::End();
}

boost::optional<nv::editor::NodeEditor&> nv::editor::NodeManager::getCurrentTab() {
	if (m_currTab) {
		return *m_currTab;
	} else {
		return boost::none;
	}
}
