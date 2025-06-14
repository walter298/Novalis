#include "App.h"

#include <concepts>
#include <chrono>
#include <fstream>
#include <stack>
#include <variant>
#include <string>
#include <boost/optional.hpp>
#include <magic_enum/magic_enum.hpp>

#include "EditedObjectData.h"
#include "ImGuiID.h"
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
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; 
	ImGui::StyleColorsDark();
	ImGui_ImplSDL3_InitForSDLRenderer(app.instance.window, app.instance.renderer);
	ImGui_ImplSDLRenderer3_Init(app.instance.renderer);

	while (app.running) {
		resetImGuiIDs();
		
		constexpr float FONT_SCALE = 3.0f;
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

		static constexpr ImVec4 color{ 0.0f, 0.0f, 0.0f, 1.00f };

		ImGui_ImplSDLRenderer3_NewFrame();
		ImGui_ImplSDL3_NewFrame();
		ImGui::NewFrame();

		app.taskBar.show(app.instance.renderer, app.tabs);
		app.toolDisplay.show(app.tabs.currentTab().has_value() && app.tabs.currentTab()->isBusy());
		app.tabs.show(app.instance.renderer, app.toolDisplay);

		const auto now = std::chrono::system_clock::now();
		if (now < endTime) {
			std::this_thread::sleep_for(endTime - now);
		}

		SDL_SetRenderScale(app.instance.renderer, io.DisplayFramebufferScale.x, io.DisplayFramebufferScale.y);
		ImGui::Render();
		ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), app.instance.renderer);
		SDL_RenderPresent(app.instance.renderer);
	}

	ImGui_ImplSDLRenderer3_Shutdown();
	ImGui_ImplSDL3_Shutdown();
	ImGui::DestroyContext();
}