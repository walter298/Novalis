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

void nv::editor::centerNextWindow() {
	auto center = ImGui::GetMainViewport()->GetCenter();
	ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2{ 0.5f, 0.5f });
}

SDL_FRect nv::editor::getWindowRect(const char* windowName) noexcept {
	auto win = ImGui::FindWindowByName(windowName);
	auto pos = win->Pos;
	auto size = win->Size;
	return { pos.x,  pos.y, size.x, size.y };
}

bool nv::editor::windowContainsCoord(const char* windowName, Point p) noexcept {
	auto winRect = getWindowRect(windowName);
	SDL_FPoint sdlP = p;
	return SDL_PointInRectFloat(&sdlP, &winRect);
}

float nv::editor::getInputWidth() noexcept {
	return ImGui::GetIO().DisplaySize.x * 0.1f;
}

nv::Point nv::editor::getViewportAdjustedMouse(SDL_Renderer* renderer, Point mouse, float zoom) noexcept {
	SDL_Rect viewport;
	SDL_GetRenderViewport(renderer, &viewport);

	mouse.x -= viewport.x;
	mouse.y -= viewport.y;
	mouse.x /= zoom;
	mouse.y /= zoom;

	return mouse;
}

nv::Point nv::editor::toSDLFPoint(ImVec2 p) noexcept {
	return { p.x, p.y };
}

SDL_FRect nv::editor::getScreenDimensions() noexcept {
	const auto& display = ImGui::GetIO().DisplaySize;
	return { 0.0f, 0.0f, display.x, display.y };
}

void nv::editor::centerNextText(const char* text) noexcept {
	auto windowWidth = ImGui::GetWindowSize().x;
	auto textWidth = ImGui::CalcTextSize(text).x;
	ImGui::SetCursorPosX((windowWidth - textWidth) * 0.5f);
}

void nv::editor::showDisabledMenu(const char* text) noexcept {
	ImGui::BeginDisabled();
	if (ImGui::BeginMenu(text)) {
		ImGui::EndMenu();
	}
	ImGui::EndDisabled();
}

void nv::editor::clampImVec2(ImVec2& v, ImVec2 maxSize) noexcept {
	v.x = std::min(v.x, maxSize.x);
	v.y = std::min(v.y, maxSize.y);
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
