#pragma once

#include <magic_enum/magic_enum.hpp>

#include "imgui/imgui.h"
#include "detail/file/File.h"
#include "Texture.h"
#include "WindowLayout.h"

namespace nv {
	namespace editor {
		enum class Tool {
			Move,
			ObjectSelect,
			ObjectGrab,
			AreaSelect,
			Polygon,
			Text,
			Delete
		};

		struct ToolTextureData {
			Texture hand;
			Texture foo;
		};

		class ToolDisplay {
			static constexpr Point TEXTURE_SIZE = { 160.0f, 160.0f };

			boost::unordered_flat_map<Tool, TexturePtr> m_buttons;
			Tool m_currTool = Tool::Move;
		public:
			class GrabberTool {
			private:
				Texture m_hand;
				Texture m_grab;
				Tool& m_tool;
			public:
				GrabberTool(Tool& tool) : m_tool{ tool } {};
				void initAfterTextureLoad(TexturePtr hand, TexturePtr grab) noexcept
				{
					m_hand = hand;
					m_grab = grab;
					m_hand.setScreenSize(TEXTURE_SIZE);
					m_grab.setScreenSize(TEXTURE_SIZE);
				}
				void render(SDL_Renderer* renderer, Point mouse) noexcept {
					auto& tex = ImGui::IsMouseDown(ImGuiMouseButton_Left) ? m_grab : m_hand;
					ImGui::SetMouseCursor(ImGuiMouseCursor_None);
					tex.setScreenPos(mouse);
					tex.render(renderer);
				}
				void grab() noexcept {
					m_tool = Tool::ObjectGrab;
				}
			};

			GrabberTool grabber;

			using Entry = std::pair<const Tool, Texture>;

			ToolDisplay(SDL_Renderer* renderer) : grabber{ m_currTool } {
				namespace fs = std::filesystem;
				for (const auto& filePath : fs::directory_iterator(workingDirectory() + "novalis_assets/tool_images")) {
					if (filePath.is_directory()) {
						continue;
					}
					auto filename = filePath.path().stem().string();
					auto tool = magic_enum::enum_cast<Tool>(filename);
					assert(tool.has_value());
					auto pathStr = filePath.path().string();

					TexturePtr tex{ renderer, pathStr.c_str() };
					m_buttons.emplace(tool.value(), std::move(tex));
				}
				grabber.initAfterTextureLoad(m_buttons.at(Tool::ObjectSelect), m_buttons.at(Tool::ObjectGrab));
			}

			void show(bool disabled) noexcept {
				ImGui::SetNextWindowPos({ 0.0f, getWindowY() });
				ImGui::SetNextWindowSize({ getSideWindowWidth(), getWindowHeight() * 0.3f });

				ImGui::BeginDisabled(disabled);
				ImGui::Begin(TOOL_WINDOW_NAME, nullptr, WINDOW_FLAGS);

				int i = 1;
				for (const auto& [tool, tex] : m_buttons) {
					if (tool == Tool::ObjectGrab) { //only show the object select
						continue;
					}
					const bool highlighted = tool == m_currTool;
					if (highlighted) {
						ImGui::PushStyleColor(ImGuiCol_Button, { 0, 255, 0, 255 });
						ImGui::PushStyleColor(ImGuiCol_ButtonHovered, { 0, 255, 0, 255 });
						ImGui::PushStyleColor(ImGuiCol_ButtonActive, { 0, 255, 0, 255 });
					}
					auto strId = std::to_string(i);
					if (ImGui::ImageButton(strId.c_str(), reinterpret_cast<ImTextureID>(tex.tex), getToolSize())) {
						m_currTool = tool;
					}
					if (highlighted) {
						ImGui::PopStyleColor();
						ImGui::PopStyleColor();
						ImGui::PopStyleColor();
					}
					if (i % 3 != 0) {
						ImGui::SameLine();
					}
					i++;
				}
				ImGui::End();
				ImGui::EndDisabled();
			}

			Tool getCurrentTool() noexcept {
				return m_currTool;
			}
		};
	}
}