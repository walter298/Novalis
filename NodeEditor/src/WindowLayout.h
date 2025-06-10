#pragma once

#include <SDL3/SDL.h>

#include "imgui/imgui.h"
#include "imgui/imgui_impl_sdlrenderer3.h"
#include "imgui/imgui_impl_sdl3.h"
#include "imgui/imgui_internal.h"
#include "imgui/imgui_stdlib.h"

namespace nv {
	namespace editor {
		inline constexpr const char* TOOL_WINDOW_NAME = "Tools";
		inline constexpr const char* OBJECT_WINDOW_NAME = "Object";
		inline constexpr const char* NODE_WINDOW_NAME = "Nodes";
		inline constexpr const char* NODE_OPTIONS_WINDOW_NAME = "Current Node";
		inline constexpr const char* LAYER_EXPLORER_WINDOW_NAME = "Layer Explorer";
		inline constexpr const char* OBJECT_GROUP_CREATION_WINDOW_NAME = "Create Object Group";
		
		constexpr ImGuiWindowFlags WINDOW_FLAGS = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse;

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

		inline float getSideWindowWidth() noexcept {
			return ImGui::GetIO().DisplaySize.x * 0.2f;
		}

		inline float getWindowY() noexcept {
			return ImGui::GetIO().DisplaySize.y * 0.05f;
		}

		inline float getWindowHeight() noexcept {
			return ImGui::GetIO().DisplaySize.y - getWindowY();
		}

		/*inline ImVec2 getToolSize() {
			return {
				getSideWindowWidth() / 3.0f,
				getSideWindowWidth() / 2.0f
			};
		}*/

		inline float getToolWindowHeight() noexcept {
			return getWindowHeight() * 0.2f;
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

		inline ImVec2 getAdjacentWindowPos(const char* adjacentWindowName) noexcept {
			auto leftWindow = ImGui::FindWindowByName(adjacentWindowName);
			assert(leftWindow);
			auto leftWindowPos = leftWindow->Pos;
			auto leftWindowSize = leftWindow->Size;
			return ImVec2{ (leftWindowPos.x + leftWindowSize.x), leftWindowPos.y };
		}

		inline ImVec2 getTabWindowPos() noexcept {
			return getAdjacentWindowPos(TOOL_WINDOW_NAME);
		}

		inline ImVec2 getTabWindowSize() noexcept {
			auto parentWindowSize = ImGui::GetIO().DisplaySize;
			return {
				parentWindowSize.x - (2.0f * getSideWindowWidth()),
				getWindowHeight()
			};
		}

		inline ImVec2 getNodeOptionsWindowPos() noexcept {
			return getAdjacentWindowPos(NODE_WINDOW_NAME);
		}

		inline SDL_FRect getViewport(float zoom) noexcept {
			auto tabWindowPos = getTabWindowPos();
			auto tabWindowSize = getTabWindowSize();
			return {
				tabWindowPos.x / zoom, tabWindowPos.y / zoom,
				tabWindowSize.x / zoom, tabWindowSize.y / zoom
			};
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
	}
}