#include "SpriteEditor.h"

#include <print>

using nv::editor::SpriteEditor;

void SpriteEditor::makeUnselectedLayersInvisible() {
	for (auto& [index, texLayer] : m_textures | views::take(m_currLayer) | views::drop(m_currLayer)) {
		for (auto& texData : texLayer) {
			texData.setOpacity(0);
		}
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
		makeUnselectedLayersInvisible();
	}

	//insert textures
	if (ImGui::Button("Insert Texture(s)")) {
		auto images = openFilePaths();
		if (images) {
			TexturePos defaultPos;
			defaultPos.ren.setSize(300, 300);
			for (const auto& image : *images) {
				m_textures[m_currLayer].emplace(
					std::make_shared<Texture>(IMG_LoadTexture(renderer.get(), image.c_str())),
					defaultPos
				);
				renderer.add(&getBack(m_textures[m_currLayer]), m_currLayer);
				defaultPos.ren.rect.x += 300;
			}
		}
	}

	if (ImGui::Button("Set Layout")) {
		//set layout
	}
	ImGui::SameLine();
	ImGui::InputInt("Target Layer", &m_currLayoutLayer);
	
	if (ImGui::Button("Save")) {

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