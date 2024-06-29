#include "SceneEditor.h"

using nv::editor::SceneEditor;

nv::Layers<nv::TextureData> nv::editor::SceneEditor::getUniqueObjectData(const LoadedSprite& sprite) {
	Layers<TextureData> spriteDataLayers;
	for (const auto& [layer, texObjs] : sprite.texObjLayers) {
		for (const auto& texObj : texObjs) {
			spriteDataLayers[layer].push_back(texObj.texData);
		}
	}
	return spriteDataLayers;
}

nv::TextureData nv::editor::SceneEditor::getUniqueObjectData(const LoadedTextureObject& texObj) {
	return texObj.texData;
}

void nv::editor::SceneEditor::loadSprite() {
	auto filePath = openFilePath();
	if (filePath) {
		try {
			std::ifstream file{ *filePath };
			auto json = json::parse(file);
			auto jsonFormat = json.get<Sprite::JsonFormat>();
			
			LoadedSprite newSprite;
			newSprite.texPaths = std::make_shared<LoadedSprite::TexturePaths>();
			for (auto& [layer, texData] : jsonFormat) {
				auto& newTexLayer = newSprite.texObjLayers[layer];
				for (auto& [texPath, texData] : texData) {
					newTexLayer.emplace_back(
						std::make_shared<TextureDestructorWrapper>(IMG_LoadTexture(m_renderer, texPath.c_str())), 
						std::move(texData)
					);
					newSprite.texPaths->push_back(std::move(texPath));
				}
			}
		} catch (json::exception e) {
			std::println("{}", e.what());
		}
	} 
}

void nv::editor::SceneEditor::createTexture() noexcept {
	auto filename = openFilePath();
	if (filename) {
		auto& texLayer = m_localTextureLayers[m_currLayer];
		texLayer.emplace_back(
			*filename, 
			std::make_shared<TextureDestructorWrapper>(IMG_LoadTexture(m_renderer, filename->c_str())), 
			TextureData{}
		);
		texLayer.back().scale(500, 500);
		m_objEditor.reseat(&texLayer, m_currLayer);
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
	if (filename) {
		json sceneJ;
		saveObjects(sceneJ, "sprites", m_spriteLayers, &LoadedSprite::texPaths, "texture_paths", "sprite_data");
		saveObjects(sceneJ, "textures", m_localTextureLayers, &LoadedTextureObject::path, "texture_path", "texture_object_data");
	}
}

void nv::editor::SceneEditor::showSceneOptions() noexcept {
	ImGui::SetNextWindowPos({ 0, 0 });
	ImGui::SetNextWindowSize({ 300, 200 });
	ImGui::Begin("Scene");
	int intLayer = static_cast<int>(m_currLayer);
	if (ImGui::InputInt("Layer", &intLayer)) {
		if (intLayer > m_currLayer) {
			intLayer = m_currLayer + 1;
		}
		makeOneLayerMoreVisible(m_spriteLayers, m_currLayer, 50);
		makeOneLayerMoreVisible(m_localTextureLayers, m_currLayer, 50);
	}
	if (ImGui::Button("Save")) {
		save();
	}
	ImGui::End();
}

SceneEditor::SceneEditor(SDL_Renderer* renderer)
	: m_renderer{ renderer }, m_objEditor{ { 0, 0 } }
{ 
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
	
	return EditorDest::None;
}

void nv::editor::SceneEditor::sdlRender() const noexcept {
	renderCopy(m_renderer, m_spriteLayers, m_localTextureLayers);
}
