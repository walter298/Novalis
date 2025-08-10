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
		inline constexpr const char* PROJECT_LOAD_POPUP_NAME = "Load Project";

		constexpr ImGuiWindowFlags DEFAULT_WINDOW_FLAGS = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse;

		void centerNextWindow();
		SDL_FRect getWindowRect(const char* windowName) noexcept;
		bool windowContainsCoord(const char* windowName, Point p) noexcept;
		float getInputWidth() noexcept;
		Point getViewportAdjustedMouse(SDL_Renderer* renderer, Point mouse, float zoom) noexcept;
		Point toSDLFPoint(ImVec2 p) noexcept;
		SDL_FRect getScreenDimensions() noexcept;
		void centerNextText(const char* text) noexcept;
		void showDisabledMenu(const char* text) noexcept;
		void clampImVec2(ImVec2& v, ImVec2 maxSize) noexcept;
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