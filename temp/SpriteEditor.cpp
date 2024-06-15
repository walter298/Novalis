#include "SpriteEditor.h"

#include <print>

using nv::editor::SpriteEditor;

void nv::editor::SpriteEditor::open(Renderer& renderer) {
	auto filePath = openFilePath();
	if (filePath) {
		try {
			Sprite::loadTextureData(*filePath, renderer.get(), [&, this](auto texPath, int layer, auto texPtr, auto texData) {
				auto& texLayer = m_textures[layer];
				texLayer.emplace(texPath, std::move(texPtr), std::move(texData));
				renderer.add(&getBack(texLayer), layer);
			});
		} catch (json::exception e) {
			std::println("{}", e.what());
		}
	}
}

void nv::editor::SpriteEditor::save() {
	auto filename = saveFile(L"Save Texture");
	if (filename) {
		Sprite::JsonFormat jsonFormat;
		for (const auto& [layer, textures] : m_textures) {
			for (const auto& tex : textures) {
				auto& value = jsonFormat[tex.path];
				value.first = layer;
				value.second.push_back(tex.texData);
			}
		}
		std::ofstream file{ *filename };
		json j = jsonFormat;
		file << j.dump(2);
		file.close();
	}
} 

void nv::editor::SpriteEditor::insertTextures(Renderer& renderer) {
	auto texPaths = openFilePaths();
	if (texPaths) {
		TextureData defaultPos;
		defaultPos.ren.setPos(400, 400);
		defaultPos.ren.setSize(300, 300);
		for (const auto& texPath : *texPaths) {
			m_textures[m_currLayer].emplace(
				texPath,
				IMG_LoadTexture(renderer.get(), texPath.c_str()),
				defaultPos
			);
			renderer.add(&getBack(m_textures[m_currLayer]), m_currLayer);
			defaultPos.ren.rect.x += 300;
		}
	}
}

void nv::editor::SpriteEditor::setIdenticalLayout() {
	auto& texsToMove = m_textures[m_currLayer];
	auto& targetTexs = m_textures[m_currLayoutLayer];

	if (texsToMove.size() != targetTexs.size()) {
		std::println("Error: cannot model layout of layers with different # of textures");
		return;
	}
	for (auto [texToMove, targetTex] : views::zip(texsToMove, targetTexs)) {
		texToMove.setPos(targetTex.getPos());
	}
}

void SpriteEditor::showSpriteOptions(Renderer& renderer) {
	static constexpr ImVec2 layerOptionPos{ 0.0f, 0.0f };
	static constexpr ImVec2 layerOptionsSize{ 200.0f, 200.0f };

	ImGui::SetNextWindowPos(layerOptionPos);
	ImGui::SetNextWindowSize(layerOptionsSize);

	ImGui::Begin("Layer");

	//select layer
	if (ImGui::InputInt("Layer", &m_currLayer)) {
		m_texDataEditor.reseat(&m_textures[m_currLayer], m_currLayer);
		makeOneLayerMoreVisible(m_textures, m_currLayer, 100);
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

SpriteEditor::SpriteEditor(Renderer& renderer) noexcept
	: m_texDataEditor{ renderer, { 0, 500 } }
{
	m_texDataEditor.reseat(&m_textures[m_currLayer], m_currLayer);
}

nv::editor::EditorDest SpriteEditor::operator()(Renderer& renderer) {
	showSpriteOptions(renderer);
	m_texDataEditor();
	return EditorDest::None;
}