#include "App.h"

#include <concepts>
#include <chrono>
#include <fstream>
#include <stack>
#include <variant>
#include <string>
#include <boost/optional.hpp>

#include <magic_enum.hpp>

#include "../imgui/imgui.h"
#include "../imgui/imgui_impl_sdlrenderer3.h"
#include "../imgui/imgui_impl_sdl3.h"
#include "../imgui/imgui_internal.h"
#include "../imgui/imgui_stdlib.h"

#include "../Instance.h"
#include "../Node.h"

#include "EditedObjectData.h"
#include "NodeEditor.h"
#include "NodeTabList.h"
#include "SpecialPoint.h"
#include "TaskBar.h"
#include "ToolDisplay.h"
#include "WindowLayout.h"

using namespace nv;
using namespace nv::editor;

struct AppData {
	nv::Instance instance{ "Novalis" };
	NodeTabList tabs;
	TaskBar taskBar;
	ToolDisplay toolDisplay{ instance.renderer };
	bool running = true;
};

void nv::editor::runApp() {
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	
	AppData app;

	auto& io = ImGui::GetIO();
	(void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; 
	ImGui::StyleColorsDark();
	ImGui_ImplSDL3_InitForSDLRenderer(app.instance.window, app.instance.renderer);
	ImGui_ImplSDLRenderer3_Init(app.instance.renderer);

	while (app.running) {
		constexpr float FONT_SCALE = 2.1f;
		ImGui::GetIO().FontGlobalScale = FONT_SCALE;

		using namespace std::literals;

		constexpr auto waitTime = 1000ms / 500;
		const auto endTime = std::chrono::system_clock::now() + waitTime;

		SDL_Event evt;
		while (SDL_PollEvent(&evt)) {
			ImGui_ImplSDL3_ProcessEvent(&evt);
			if (evt.type == SDL_EVENT_QUIT) {
				app.running = false;
			}
		}

		SDL_RenderClear(app.instance.renderer);
		
		static constexpr ImVec4 color{ 0.45f, 0.55f, 0.60f, 1.00f };

		ImGui_ImplSDLRenderer3_NewFrame();
		ImGui_ImplSDL3_NewFrame();
		ImGui::NewFrame();

		app.taskBar.show(app.instance.renderer, app.tabs);
		app.toolDisplay.show(app.tabs.makingPolygon());
		app.tabs.show(app.instance.renderer, app.toolDisplay);

		const auto now = std::chrono::system_clock::now();
		if (now < endTime) {
			std::this_thread::sleep_for(endTime - now);
		}

		SDL_SetRenderDrawColor(app.instance.renderer,
			static_cast<uint8_t>(color.x * 255), static_cast<uint8_t>(color.y * 255),
			static_cast<uint8_t>(color.z * 255), static_cast<uint8_t>(color.w * 255));
		SDL_SetRenderScale(app.instance.renderer, io.DisplayFramebufferScale.x, io.DisplayFramebufferScale.y);
		ImGui::Render();
		ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), app.instance.renderer);
		SDL_RenderPresent(app.instance.renderer);
	}

	ImGui_ImplSDLRenderer3_Shutdown();
	ImGui_ImplSDL3_Shutdown();
	ImGui::DestroyContext();
}