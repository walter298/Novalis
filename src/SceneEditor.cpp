#include "SceneEditor.h"

using nv::editor::SceneEditor;

void nv::editor::SceneEditor::loadSprite() {
	auto filePath = openFilePath();
	if (!filePath) {
		return;
	}
	try {
		std::ifstream file{ *filePath };
		auto json = json::parse(file);
		m_spriteLayers[m_currLayer].emplace_back(m_renderer, json, m_texMap);
	} catch (json::exception e) {
		std::println("{}", e.what());
	}
}

void nv::editor::SceneEditor::createTexture() noexcept {
	auto filename = openFilePath();
	if (filename) {
		auto& texLayer = m_texObjLayers[m_currLayer];
		texLayer.emplace_back(
			*filename, 
			std::make_shared<TextureRAII>(IMG_LoadTexture(m_renderer, filename->c_str())), 
			TextureData{}
		);
		texLayer.back().obj.scale(500, 500);
	}
}

void SceneEditor::showRightClickOptions() noexcept {
	static constexpr ImVec2 btnSize{ 210.0f, 60.0f };
	static constexpr auto winSize = buttonList(btnSize, 2);

	ImGui::SetNextWindowPos(m_rightClickWinPos);
	ImGui::SetNextWindowSize(winSize);
	ImGui::Begin("Options");
	if (ImGui::Button("Upload Sprite", btnSize)) {
		loadSprite();
		m_showingRightClickOptions = false;
	}
	if (ImGui::Button("Create Texture", btnSize)) {
		createTexture();
		m_showingRightClickOptions = false;
	}
	ImGui::End();
}

void nv::editor::SceneEditor::save() noexcept {
	auto filename = saveFile(L"Save File");
	if (!filename) {
		return;
	}

	json root;

	auto spritesJson = json::array();
	auto texObjsJson = json::array();

	saveObjects(m_spriteLayers, spritesJson);
	saveObjects(m_texObjLayers, texObjsJson);

	root["sprites"]         = std::move(spritesJson);
	root["texture_objects"] = std::move(texObjsJson);
	
	std::ofstream file{ *filename };
	file << root.dump(2);
	file.close();
}

void nv::editor::SceneEditor::showSceneOptions() noexcept {
	ImGui::SetNextWindowPos({ 0, 0 });
	ImGui::SetNextWindowSize({ 150, 100 });
	ImGui::Begin("Scene");
	if (ImGui::InputInt("Layer", &m_currLayer)) {
		makeOneLayerMoreVisible(m_spriteLayers, m_currLayer, 50);
	}
	if (ImGui::Button("Save")) {
		save();
	}
	ImGui::End();
}

void nv::editor::SceneEditor::editLayers() {
	ImGui::SetNextWindowSize({ 300, 200 });
	ImGui::SetNextWindowPos({ 0, 160 });
	switch (m_selectedObjType) {
	case SelectedObjectType::Sprite:
		ImGui::Begin("Sprite");
		edit(m_selectedSpriteData);
		ImGui::End();
		break;
	case SelectedObjectType::Texture:
		ImGui::Begin("Texture Object");
		edit(m_selectedTextureData);
		ImGui::End();
		break;
	}

	auto select = [this](auto& objLayer, auto& selectedObjData, SelectedObjectType objType) {
		if (ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
			auto selectedObj = selectObj(objLayer, convertPair<SDL_Point>(ImGui::GetMousePos()));
			if (selectedObj != objLayer.end()) {
				selectedObjData.obj = &(*selectedObj);
				selectedObjData.objLayer = &objLayer;
				selectedObjData.it = selectedObj;
				m_selectedObjType = objType;
			}
		}
	};
	select(m_spriteLayers[m_currLayer], m_selectedSpriteData, SelectedObjectType::Sprite);
	select(m_texObjLayers[m_currLayer], m_selectedTextureData, SelectedObjectType::Texture);
}

SceneEditor::SceneEditor(SDL_Renderer* renderer)
	: m_renderer{ renderer }
{ 
}

nv::editor::EditorDest SceneEditor::imguiRender() {
	showSceneOptions();
	if (ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
		m_showingRightClickOptions = true;
		m_rightClickWinPos = ImGui::GetMousePos();
	}
	if (m_showingRightClickOptions) {
		showRightClickOptions();
	}
	editLayers();
	return EditorDest::None;
}

void nv::editor::SceneEditor::sdlRender() const noexcept {
	renderCopy(m_renderer, m_texObjLayers, m_spriteLayers);
}
