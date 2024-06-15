#include "Sprite.h"

nv::Sprite::Sprite(std::string_view path, SDL_Renderer* renderer) noexcept {
	loadTextureData(path, renderer, [this](auto texPath, int layer, auto texPtr, auto texData) {
		m_textureLayers[layer].emplace_back(std::move(texPtr), std::move(texData));
	});
}

void nv::Sprite::setLayer(int layer) noexcept {
	m_currLayer = layer;
}

nv::TextureData& nv::Sprite::texData(size_t texIdx) {
	return m_textureLayers[m_currLayer][texIdx].texData;
}

void nv::Sprite::setPos(int destX, int destY) noexcept {
	for (auto& [layer, textures] : m_textureLayers) {
		auto [x, y] = textures[0].getPos();
		SDL_Point change{ destX - x, destY - y };
		for (auto& tex : textures) {
			tex.move(change);
		}
	}
}

void nv::Sprite::setPos(SDL_Point p) noexcept {
	setPos(p.x, p.y);
}

void nv::Sprite::move(int x, int y) noexcept {
	for (auto& texData : m_textureLayers.at(m_currLayer)) {
		texData.move(x, y);
	}
}

void nv::Sprite::move(SDL_Point p) noexcept {
	move(p.x, p.y);
}

void nv::Sprite::scale(int x, int y) noexcept {
	for (auto& texData : m_textureLayers.at(m_currLayer)) {
		texData.scale(x, y);
	}
}

void nv::Sprite::scale(SDL_Point p) noexcept {
	scale(p.x, p.y);
}

//don't know how the hell this will work
void nv::Sprite::rotate(double angle, SDL_Point p) {}

void nv::Sprite::setRotationCenter() noexcept {}

bool nv::Sprite::containsCoord(int x, int y) const noexcept {
	return ranges::any_of(m_textureLayers.at(m_currLayer), [&](const auto& texData) { 
		return texData.containsCoord(x, y);
	});
}

bool nv::Sprite::containsCoord(SDL_Point p) const noexcept {
	return containsCoord(p.x, p.y);
}

void nv::Sprite::render(SDL_Renderer* renderer) const noexcept {
	if constexpr (true) {
		if (!m_textureLayers.contains(m_currLayer)) {
			return;
		}
	}
	for (const auto& texData : m_textureLayers.at(m_currLayer)) {
		texData.render(renderer);
	}
}
