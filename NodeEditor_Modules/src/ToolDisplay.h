#pragma once

#include <cstdlib>
#include <magic_enum/magic_enum.hpp>
#include <novalis/detail/file/File.h>
#include <novalis/Texture.h>

#include "imgui/imgui.h"
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
		private:
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
				static constexpr const char* rootDirEnvVar = "NOVALIS_ROOT";
				auto rootDirPath = std::getenv(rootDirEnvVar);
				if (!rootDirPath) {
					std::println(stderr, "Error: {} environment variable not set. Should be set to parent of NodeEditor and Library subdirectories.", rootDirEnvVar);
					std::exit(EXIT_FAILURE);
				}

				namespace fs = std::filesystem;
				std::println("{}", (fs::path{ rootDirPath } / "NodeEditor/novalis_assets/tool_images").string());

				fs::directory_iterator toolImageDir{ 
					fs::path{ rootDirPath } / "NodeEditor/novalis_assets/tool_images"
				};
				for (const auto& toolImagePath : toolImageDir) {
					auto filename = toolImagePath.path().stem().string();
					auto tool = magic_enum::enum_cast<Tool>(filename);
					assert(tool.has_value());
					TexturePtr tex{ renderer, toolImagePath.path().string().c_str() };
					m_buttons.emplace(tool.value(), std::move(tex));
				}
				grabber.initAfterTextureLoad(m_buttons.at(Tool::ObjectSelect), m_buttons.at(Tool::ObjectGrab));
			}

			void show(bool disabled) noexcept {
				const ImVec2 winSize{
					getSideWindowWidth(),
					getWindowHeight() * 0.3f
				};
				const ImVec2 toolButtonSize{
					winSize.x / 3.3f,
					winSize.y / 2.5f
				};
				
				ImGui::SetNextWindowPos({ 0.0f, getWindowY() });
				ImGui::SetNextWindowSize(winSize);

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
					if (ImGui::ImageButton(strId.c_str(), reinterpret_cast<ImTextureID>(tex.tex), toolButtonSize)) {
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