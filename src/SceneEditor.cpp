#include "SceneEditor.h"

using nv::editor::SceneEditor;

void nv::editor::SceneEditor::createTexture() noexcept {
	auto filename = openFilePath();
	if (filename) {
		/*auto& texLayer = m_localTextureLayers[m_currLayer];
		texLayer.emplace(*filename, std::make_shared<Texture>(IMG_LoadTexture(renderer.get(), filename->c_str())), TextureData{});
		auto& insertedTex = getBack(texLayer);
		insertedTex.scale(500, 500);
		renderer.add(&insertedTex, m_currLayer);*/
		//m_objEditor.reseat(&texLayer, m_currLayer);
	}
}

void SceneEditor::showRightClickOptions() noexcept {
	static constexpr ImVec2 btnSize{ 210.0f, 60.0f };
	static constexpr auto winSize = buttonList(btnSize, 3);

	ImGui::SetNextWindowPos(m_rightClickWinPos);
	ImGui::SetNextWindowSize(winSize);
	ImGui::Begin("Options");
	/*if (ImGui::Button("Upload Sprite", btnSize)) {
		insertObjFromFile(renderer, m_spriteLayers[m_currLayer]);
		m_showingRightClickOptions = false;
	}
	if (ImGui::Button("Upload Texture", btnSize)) {
		insertObjFromFile(renderer, m_textureLayers[m_currLayer]);
		m_showingRightClickOptions = false;
	}*/
	if (ImGui::Button("Create Texture", btnSize)) {
		createTexture();
		m_showingRightClickOptions = false;
	}
	ImGui::End();
}

void nv::editor::SceneEditor::save() noexcept {
	auto filename = saveFile(L"Save File");
	if (filename) {
		json j;
		
	}
}

void nv::editor::SceneEditor::showSceneOptions() noexcept {
	ImGui::SetNextWindowPos({ 0, 0 });
	ImGui::SetNextWindowSize({ 300, 200 });
	ImGui::Begin("Scene");
	if (ImGui::InputInt("Layer", &m_currLayer)) {
		/*makeOneLayerMoreVisible(m_spriteLayers, m_currLayer, 50);
		makeOneLayerMoreVisible(m_textureLayers, m_currLayer, 50);
		makeOneLayerMoreVisible(m_localTextureLayers, m_currLayer, 50);*/
	}
	if (ImGui::Button("Save")) {

	}
	ImGui::End();
}

void SceneEditor::moveCamera(SDL_Renderer* renderer) noexcept {
	static constexpr int CAMERA_DELTA = 15;
	/*if (ImGui::IsKeyPressed(ImGuiKey_RightArrow)) {
		renderer.move(-CAMERA_DELTA, 0);
	} else if (ImGui::IsKeyPressed(ImGuiKey_LeftArrow)) {
		renderer.move(CAMERA_DELTA, 0);
	} else if (ImGui::IsKeyPressed(ImGuiKey_UpArrow)) {
		renderer.move(0, CAMERA_DELTA);
	} else if (ImGui::IsKeyPressed(ImGuiKey_DownArrow)) {
		renderer.move(0, -CAMERA_DELTA);
	}*/
}

SceneEditor::SceneEditor(SDL_Renderer* renderer)
	: m_renderer{ renderer }, m_objEditor { { 0, 0 }
}
{ 
	m_objEditor.reseat(&m_spriteLayers[0], 0);
}

nv::editor::EditorDest SceneEditor::imguiRender() {
	m_objEditor();

	showSceneOptions();
	if (ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
		m_showingRightClickOptions = true;
		m_rightClickWinPos = ImGui::GetMousePos();
	}
	if (m_showingRightClickOptions) {
		showRightClickOptions();
	}
	moveCamera(m_renderer);

	return EditorDest::None;
}

void nv::editor::SceneEditor::sdlRender() const noexcept {
	renderCopy(m_renderer, m_spriteLayers, m_textureLayers);
}
