#include "EditorApp.h"

void nv::editor::runEditors() {
	Instance instance{ "Novalis" };

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	auto& io = ImGui::GetIO();
	(void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	ImGui::StyleColorsDark();
	ImGui_ImplSDL2_InitForSDLRenderer(instance.getRawWindow(), instance.getRawRenderer());
	ImGui_ImplSDLRenderer2_Init(instance.getRawRenderer());

	bool running = true;
	Renderer renderer{ instance.getRawRenderer() };
	EditorDest currDest = EditorDest::Home;
	Texture m_xBtn{ IMG_LoadTexture(renderer.get(), relativePath("Cosmic_Encounter/src/novalis/assets/x_btn.png").c_str()) };
	while (running) {
		if (currDest == EditorDest::None) {
			continue;
		}
		switch (currDest) {
		case EditorDest::Home:
			currDest = runEditor(io, renderer, runHomeEditor);
			break;
		case EditorDest::Sprite:
		{ 
			SpriteEditor spriteEditor{ renderer };
			currDest = runEditor(io, renderer, spriteEditor);
			break;
		}
		case EditorDest::Scene:
		{
			SceneEditor sceneEditor{ renderer };
			currDest = runEditor(io, renderer, sceneEditor);
			break;
		}
		case EditorDest::Quit:
			running = false;
			break;
		}
		renderer.clear();
	}

	renderer.clear();

	ImGui_ImplSDLRenderer2_Shutdown();
	ImGui_ImplSDL2_Shutdown();
	ImGui::DestroyContext();
}