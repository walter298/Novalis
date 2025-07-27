#pragma once

#include <novalis/Point.h>
#include <SDL3/SDL.h>

#include "imgui/imgui.h"
#include "imgui/imgui_impl_sdlrenderer3.h"
#include "imgui/imgui_impl_sdl3.h"
#include "imgui/imgui_internal.h"
#include "imgui/imgui_stdlib.h"

namespace nv {
	namespace editor {
		inline constexpr const char* TOOL_WINDOW_NAME = "Tools";
		inline constexpr const char* CHILD_NODE_WINDOW_NAME = "Object";
		inline constexpr const char* TAB_WINDOW_NAME = "Nodes";
		inline constexpr const char* NODE_OPTIONS_WINDOW_NAME = "Current Node";
		inline constexpr const char* LAYER_EXPLORER_WINDOW_NAME = "Layer Explorer";
		inline constexpr const char* OBJECT_GROUP_CREATION_WINDOW_NAME = "Create Object Group";
		inline constexpr const char* LAYER_CREATION_POPUP_NAME = "Create Layer";
		inline constexpr const char* NODE_CREATION_POPUP_NAME = "Create Node";
		inline constexpr const char* SPRITESHEET_CREATION_POPUP_NAME = "Create Spritesheet";
		inline constexpr const char* ERROR_POPUP_NAME = "Error Window";
		inline constexpr const char* PROJECT_CREATION_POPUP_NAME = "Create Project";
		inline constexpr const char* FILESYSTEM_WINDOW_NAME = "Filesystem";
		inline constexpr const char* FILESYSTEM_DIALOG_POPUP_NAME = "File Dialog";

		constexpr ImGuiWindowFlags DEFAULT_WINDOW_FLAGS = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse;

		inline void centerNextWindow() {
			auto center = ImGui::GetMainViewport()->GetCenter();
			ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2{ 0.5f, 0.5f });
		}

		inline SDL_FRect getWindowRect(const char* windowName) noexcept {
			auto win = ImGui::FindWindowByName(windowName);
			auto pos = win->Pos;
			auto size = win->Size;
			return { pos.x,  pos.y, size.x, size.y };
		}

		inline bool windowContainsCoord(const char* windowName, Point p) noexcept {
			auto winRect = getWindowRect(windowName);
			SDL_FPoint sdlP = p;
			return SDL_PointInRectFloat(&sdlP, &winRect);
		}

		inline float getInputWidth() noexcept {
			return ImGui::GetIO().DisplaySize.x * 0.1f;
		}

		inline Point getViewportAdjustedMouse(SDL_Renderer* renderer, Point mouse, float zoom) noexcept {
			SDL_Rect viewport;
			SDL_GetRenderViewport(renderer, &viewport);

			mouse.x -= viewport.x;
			mouse.y -= viewport.y;
			mouse.x /= zoom;
			mouse.y /= zoom;

			return mouse;
		}

		inline Point toSDLFPoint(ImVec2 p) noexcept {
			return { p.x, p.y };
		}

		inline SDL_FRect getScreenDimensions() noexcept {
			const auto& display = ImGui::GetIO().DisplaySize;
			return { 0.0f, 0.0f, display.x, display.y };
		}

		inline void centerNextText(const char* text) noexcept {
			auto windowWidth = ImGui::GetWindowSize().x;
			auto textWidth = ImGui::CalcTextSize(text).x;
			ImGui::SetCursorPosX((windowWidth - textWidth) * 0.5f);
		}

		inline void showDisabledMenu(const char* text) noexcept {
			ImGui::BeginDisabled();
			if (ImGui::BeginMenu(text)) {
				ImGui::EndMenu();
			}
			ImGui::EndDisabled();
		}

		inline void clampImVec2(ImVec2& v, ImVec2 maxSize) noexcept {
			v.x = std::min(v.x, maxSize.x);
			v.y = std::min(v.y, maxSize.y);
		}

		ImVec2 getFilesystemWindowPos() noexcept;
		ImVec2 getFilesystemWindowSize() noexcept;
		ImVec2 getToolWindowPos() noexcept;
		ImVec2 getToolWindowSize() noexcept;
		ImVec2 getTabWindowPos() noexcept;
		ImVec2 getTabWindowSize() noexcept;
		ImVec2 getChildNodeWindowPos() noexcept;
		ImVec2 getChildNodeWindowSize() noexcept;
		ImVec2 getNodeOptionsWindowPos() noexcept;
		ImVec2 getNodeOptionsWindowSize() noexcept;
		ImVec2 getErrorWindowPos() noexcept;
		ImVec2 getErrorWindowSize() noexcept;
		SDL_FRect getViewport(float zoom) noexcept;
	}
}