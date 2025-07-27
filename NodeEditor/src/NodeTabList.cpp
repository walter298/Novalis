//#include "NodeTabList.h"
//#include "WindowLayout.h"
//
//boost::optional<nv::editor::NodeEditor&> nv::editor::showTabBar(plf::hive<NodeEditor>& tabs) {
//	auto tabWindowPos = getTabWindowPos();
//	auto tabWindowSize = getTabWindowSize();
//	ImGui::SetNextWindowPos(tabWindowPos);
//	ImGui::SetNextWindowSize(tabWindowSize);
//
//	ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4{ 0.0f, 0.0f, 0.0f, 0.f });
//
//	ImGui::Begin(TAB_WINDOW_NAME, nullptr, DEFAULT_WINDOW_FLAGS);
//
//	NodeEditor* selectedNodeEditor = nullptr;
//	bool deletedTab = false;
//
//	if (ImGui::BeginTabBar("Tabs")) {
//		for (auto it = tabs.begin(); it != tabs.end(); it++) {
//			ImGui::PushID(getUniqueImGuiID());
//			auto& tab = *it;
//
//			if (ImGui::BeginTabItem(tab.getName())) {
//				selectedNodeEditor = &tab;
//				ImGui::Text(tab.getName());
//				ImGui::EndTabItem();
//			}
//			ImGui::PopID();
//		}
//		ImGui::EndTabBar();
//	}
//
//	ImGui::PopStyleColor();
//
//	ImGui::End();
//
//	if (selectedNodeEditor) {
//		return *selectedNodeEditor;
//	} else {
//		return boost::none;
//	}
//}