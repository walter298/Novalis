#include "SpriteEditor.h"

using nv::editor::SpriteEditor;

void nv::editor::SpriteEditor::open(SDL_Renderer* renderer) {
	auto filePath = openFilePath();
	if (filePath) {
		try {
			m_texLayers.clear();

			std::ifstream file{ *filePath };
			auto json = json::parse(file);
			auto texJsonData = json.get<Sprite::JsonFormat>();
			m_texLayers.reserve(texJsonData.size());

			for (auto& [layer, texObjs] : texJsonData) {
				m_texLayers.emplace();
				auto& back = getBack(m_texLayers).second;
				back.reserve(texObjs.size());
				for (auto& [texPath, texData] : texObjs) {
					back.emplace_back(texPath, std::make_shared<TextureDestructorWrapper>(IMG_LoadTexture(renderer, texPath.c_str())), std::move(texData));
				}
			}
		} catch (json::exception e) {
			std::println("{}", e.what());
		}
	}
}

void nv::editor::SpriteEditor::save() {
	auto filename = saveFile(L"Save Texture");
	if (filename) {
		Sprite::JsonFormat spriteJson;
		for (const auto& [layer, texObjs] : m_texLayers) {
			spriteJson.emplace();
			auto& currDataLayer = getBack(spriteJson).second;
			for (const auto& tex : texObjs) {
				currDataLayer.emplace_back(*tex.path, tex.texData);
			}
		}
		std::ofstream file{ *filename };
		file << json{ spriteJson }.dump(2);
		file.close();
	}
} 

void nv::editor::SpriteEditor::insertTextures(SDL_Renderer* renderer) {
	auto texPaths = openFilePaths();
	if (texPaths) {
		TextureData defaultPos;
		defaultPos.ren.setPos(400, 400);
		defaultPos.ren.setSize(300, 300);
		for (const auto& texPath : *texPaths) {
			m_texLayers[m_currLayer].emplace_back(
				texPath,
				std::make_shared<TextureDestructorWrapper>(IMG_LoadTexture(renderer, texPath.c_str())),
				defaultPos
			);
			defaultPos.ren.rect.x += 300;
		}
	}
}

void nv::editor::SpriteEditor::setIdenticalLayout() {
	auto& texsToMove = m_texLayers[m_currLayer];
	auto& targetTexs = m_texLayers[m_currLayoutLayer];

	if (texsToMove.size() != targetTexs.size()) {
		std::println("Error: cannot model layout of layers with different # of textures");
		return;
	}
	for (auto [texToMove, targetTex] : views::zip(texsToMove, targetTexs)) {
		texToMove.setPos(targetTex.getPos());
	}
}

void SpriteEditor::showSpriteOptions(SDL_Renderer* renderer) {
	static constexpr ImVec2 layerOptionPos{ 0.0f, 0.0f };
	static constexpr ImVec2 layerOptionsSize{ 200.0f, 200.0f };

	ImGui::SetNextWindowPos(layerOptionPos);
	ImGui::SetNextWindowSize(layerOptionsSize);

	ImGui::Begin("Layer");

	//select layer
	if (ImGui::InputInt("Layer", &m_currLayer)) {
		m_texDataEditor.reseat(&m_texLayers[m_currLayer], m_currLayer);
		makeOneLayerMoreVisible(m_texLayers, m_currLayer, 100);
	}

	//insert textures
	if (ImGui::Button("Insert Texture(s)")) {
		insertTextures(renderer);
	}

	if (ImGui::Button("Set Layout")) {
		setIdenticalLayout();
	}
	ImGui::SameLine();
	ImGui::InputInt("Target Layer", &m_currLayoutLayer);
	
	if (ImGui::Button("Save")) {
		save();
	}
	ImGui::SameLine();
	if (ImGui::Button("Open")) {
		open(renderer);
	}
	ImGui::End();
}

SpriteEditor::SpriteEditor(SDL_Renderer* renderer) noexcept
	: m_renderer{ renderer }, m_texDataEditor { { 0, 500 } }
{
}

nv::editor::EditorDest SpriteEditor::imguiRender() {
	showSpriteOptions(m_renderer);
	m_texDataEditor();
	return EditorDest::None;
}

void nv::editor::SpriteEditor::sdlRender() noexcept {
	renderCopy(m_renderer, m_texLayers);
}
