#include "WindowLayout.h"

static float getSideWindowWidth() noexcept {
	return ImGui::GetIO().DisplaySize.x * 0.2f;
}

static float getWindowY() noexcept {
	return ImGui::GetIO().DisplaySize.y * 0.05f;
}

static float getWindowHeight() noexcept {
	return ImGui::GetIO().DisplaySize.y - getWindowY();
}

ImVec2 nv::editor::getToolWindowPos() noexcept {
	return { 0.0f, getWindowY() };
}

ImVec2 nv::editor::getToolWindowSize() noexcept {
	return { getSideWindowWidth(), getWindowHeight() * 0.3f };
}

//position below the tool window
ImVec2 nv::editor::getFilesystemWindowPos() noexcept {
	auto toolWindowPos = getToolWindowPos();
	auto toolWindowSize = getToolWindowSize();
	return { 0.0f, toolWindowPos.y + toolWindowSize.y + 0.5f };
}

ImVec2 nv::editor::getFilesystemWindowSize() noexcept {
	return { getSideWindowWidth(), getWindowHeight() * 0.7f };
}

//position next to the tool window
ImVec2 nv::editor::getTabWindowPos() noexcept {
	auto leftWindowPos = getToolWindowPos();
	auto leftWindowSize = getToolWindowSize();
	return ImVec2{ (leftWindowPos.x + leftWindowSize.x), leftWindowPos.y };
}

ImVec2 nv::editor::getTabWindowSize() noexcept {
	auto parentWindowSize = ImGui::GetIO().DisplaySize;
	return {
		parentWindowSize.x - (2.0f * getSideWindowWidth()),
		getWindowHeight() * 0.8f
	};
}

ImVec2 nv::editor::getNodeOptionsWindowPos() noexcept {
	auto leftWindowPos = getTabWindowPos();
	auto leftWindowSize = getTabWindowSize();
	return ImVec2{ (leftWindowPos.x + leftWindowSize.x), leftWindowPos.y };
}

ImVec2 nv::editor::getNodeOptionsWindowSize() noexcept {
	return { getSideWindowWidth(), getWindowHeight() * 0.5f };
}

ImVec2 nv::editor::getErrorWindowPos() noexcept {
	auto [tabWinX, tabWinY] = getTabWindowPos();
	auto [tabW, tabH] = getTabWindowSize();
	return { tabWinX, tabWinY + tabH };
}

ImVec2 nv::editor::getErrorWindowSize() noexcept {
	auto [tabW, tabH] = getTabWindowSize();
	return { tabW, getWindowHeight() * 0.2f };
}

ImVec2 nv::editor::getChildNodeWindowPos() noexcept {
	auto nodeWindowPos = getNodeOptionsWindowPos();
	auto nodeWindowSize = getNodeOptionsWindowSize();
	return { nodeWindowPos.x, nodeWindowPos.y + nodeWindowSize.y + 0.5f };
}

ImVec2 nv::editor::getChildNodeWindowSize() noexcept {
	return { getSideWindowWidth(), getWindowHeight() * 0.5f };
}

SDL_FRect nv::editor::getViewport(float zoom) noexcept {
	auto tabWindowPos = getTabWindowPos();
	auto tabWindowSize = getTabWindowSize();
	return {
		tabWindowPos.x / zoom, tabWindowPos.y / zoom,
		tabWindowSize.x / zoom, tabWindowSize.y / zoom
	};
}
