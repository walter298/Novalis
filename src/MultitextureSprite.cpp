#include "MultitextureSprite.h"

nv::MultitextureSprite::MultitextureSprite(const std::string& path, SDL_Renderer* renderer) noexcept {
	std::ifstream file{ path };
	auto json = json::parse(file);
	file.close();
	
	m_textures->reserve(json.at(textureSizeJkey).get<size_t>());

	using StringRenPairs = std::vector<std::pair<std::string, TexturePos>>;
	auto imagePathsAndRens = json.at(textureRenPairsJkey).get<StringRenPairs>();

	for (auto& [texPath, texturePos] : imagePathsAndRens) {
		m_textures->emplace_back(IMG_LoadTexture(renderer, texPath.c_str()), std::move(texturePos));
	}
	m_layerIdxs = json.at(layerIdxsJkey).get<std::vector<size_t>>();
}

void nv::MultitextureSprite::render(SDL_Renderer* renderer) const noexcept {
	auto nextLayerIdx = (m_currLayerIdx + 1 == m_layerIdxs.size()) ? 0 : m_currLayerIdx + 1;
	auto idxRange = views::drop(m_currLayerIdx) | views::drop(nextLayerIdx);
	for (const auto& [tex, pos] : *m_textures | idxRange) {
		SDL_RenderCopy(renderer, tex.raw, nullptr, &pos.ren.rect);
	}
}

void nv::MultitextureSprite::renMove(int dx, int dy) noexcept {
	for (auto& [tex, pos] : *m_textures) {
		pos.ren.move(dx, dy);
	}
}