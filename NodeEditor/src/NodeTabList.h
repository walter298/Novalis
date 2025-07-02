#pragma once

#include <cassert>
#include <boost/optional.hpp>

#include "NodeEditor.h"

namespace nv {
	namespace editor {
		class NodeTabList {
		private:
			plf::hive<NodeEditor> m_tabs;
			NodeEditor* m_currTab = nullptr;

			ImVec2 getTabWindowPos() noexcept {
				auto leftWindow = ImGui::FindWindowByName(TOOL_WINDOW_NAME);
				assert(leftWindow);
				auto leftWindowPos = leftWindow->Pos;
				auto leftWindowSize = leftWindow->Size;
				return ImVec2{ (leftWindowPos.x + leftWindowSize.x), leftWindowPos.y };
			}

			ImVec2 getTabWindowSize() {
				auto parentWindowSize = ImGui::GetIO().DisplaySize;
				return {
					parentWindowSize.x - (2.0f * getSideWindowWidth()),
					getWindowHeight()
				};
			}

			void showTabBar() {
				auto tabWindowPos = getTabWindowPos();
				auto tabWindowSize = getTabWindowSize();

				ImGui::SetNextWindowPos(tabWindowPos);
				ImGui::SetNextWindowSize(tabWindowSize);

				ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4{ 0.0f, 0.0f, 0.0f, 0.f });

				ImGui::Begin(NODE_WINDOW_NAME, nullptr, WINDOW_FLAGS);

				bool deletedTab = false;

				if (ImGui::BeginTabBar("Tabs")) {
					for (auto it = m_tabs.begin(); it != m_tabs.end(); it++) {
						ImGui::PushID(getUniqueImGuiID());
						auto& tab = *it;

						if (ImGui::BeginTabItem(tab.getName())) {
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
		public:
			void show(SDL_Renderer* renderer, ToolDisplay& toolDisplay) noexcept {
				showTabBar();
				if (m_currTab != nullptr) {
					m_currTab->show(renderer, toolDisplay);
				}
			}

			NodeEditor& add(const std::string& name) {
				return *m_tabs.emplace(name.c_str());
			}

			void upload() {
				auto nodeEditor = NodeEditor::load();
				if (nodeEditor) {
					m_currTab = &(*m_tabs.insert(std::move(*nodeEditor)));
				}
			}

			boost::optional<NodeEditor&> currentTab() {
				if (m_currTab) {
					return *m_currTab;
				} else {
					return boost::none;
				}
			}

			bool empty() const noexcept {
				return m_tabs.empty();
			}
		};
	}
}