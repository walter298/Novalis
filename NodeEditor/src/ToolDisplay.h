#pragma once

#include <boost/unordered/unordered_flat_map.hpp>
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

			boost::unordered_flat_map<Tool, nv::detail::TexturePtr> m_buttons;
			Tool m_currTool = Tool::Move;
		public:
			class GrabberTool {
			private:
				Texture m_hand;
				Texture m_grab;
				Tool& m_tool;
			public:
				GrabberTool(Tool& tool) : m_tool{ tool } {};
				void initAfterTextureLoad(nv::detail::TexturePtr hand, nv::detail::TexturePtr grab) noexcept
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

			ToolDisplay(SDL_Renderer* renderer);

			void show(bool disabled) noexcept;

			Tool getCurrentTool() noexcept {
				return m_currTool;
			}
		};
	}
}