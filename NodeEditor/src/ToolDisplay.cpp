#include <cstdlib>
#include <filesystem>
#include <print>
#include <magic_enum/magic_enum.hpp>
#include <novalis/detail/file/File.h>

#include "NovalisRoot.h"
#include "ToolDisplay.h"

namespace {
	using namespace nv;
	using namespace editor;

	constexpr Point TEXTURE_SIZE = { 160.0f, 160.0f };
	boost::unordered_flat_map<Tool, nv::detail::TexturePtr> m_buttons;
	Tool currTool;
}

void nv::editor::loadToolTextures(SDL_Renderer* renderer) {
	std::filesystem::directory_iterator toolImageDir{
		getNovalisRoot() / "NodeEditor/novalis_assets/tool_images"
	};
	for (const auto& toolImagePath : toolImageDir) {
		auto filename = toolImagePath.path().stem().string();
		auto tool = magic_enum::enum_cast<Tool>(filename);
		assert(tool.has_value());
		nv::detail::TexturePtr tex{ renderer, toolImagePath.path().string().c_str() };
		m_buttons.emplace(tool.value(), std::move(tex));
	}
}

void nv::editor::destroyToolTextures(SDL_Renderer* renderer) {
	for (auto& [tool, tex] : m_buttons) {
		tex.destroy();
	}
}

void nv::editor::showToolDisplay(bool disabled) {
	auto winSize = getToolWindowSize();
	const ImVec2 toolButtonSize{
		winSize.x / 3.3f,
		winSize.y / 2.5f
	};

	ImGui::SetNextWindowPos(getToolWindowPos());
	ImGui::SetNextWindowSize(winSize);

	ImGui::BeginDisabled(disabled);
	ImGui::Begin(TOOL_WINDOW_NAME, nullptr, DEFAULT_WINDOW_FLAGS);

	int i = 1;
	for (const auto& [tool, tex] : m_buttons) {
		if (tool == Tool::ObjectGrab) { //only show the object select
			continue;
		}
		const bool highlighted = (tool == currTool);
		if (highlighted) {
			ImGui::PushStyleColor(ImGuiCol_Button, { 0, 255, 0, 255 });
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, { 0, 255, 0, 255 });
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, { 0, 255, 0, 255 });
		}
		auto strId = std::to_string(i);
		if (ImGui::ImageButton(strId.c_str(), reinterpret_cast<ImTextureID>(tex.tex), toolButtonSize)) {
			currTool = tool;
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

Tool nv::editor::getCurrentTool() {
	return currTool;
}
