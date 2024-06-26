#include "Sprite.h"

nv::Sprite::Sprite(std::string_view path, SDL_Renderer* renderer) noexcept {
	std::ifstream file{ path.data() };
	auto json = json::parse(file);
	auto texJsonData = json.get<JsonFormat>();
	texObjLayers.reserve(texJsonData.size()); 

	for (auto& texLayer : texJsonData) {
		auto& back = texObjLayers.emplace_back();
		back.reserve(texLayer.size());
		for (auto& [texPath, texData] : texLayer) {
			back.emplace_back(std::make_shared<TextureDestructorWrapper>(IMG_LoadTexture(renderer, texPath.c_str())), std::move(texData));
		}
	}
}

nv::TextureData& nv::Sprite::texData(size_t texIdx) {
	return texObjLayers[currLayer][texIdx].texData;
}

void nv::Sprite::setPos(int destX, int destY) noexcept {
	for (auto& texLayer : texObjLayers) {
		auto [x, y] = texLayer[0].getPos();
		SDL_Point change{ destX - x, destY - y };
		for (auto& tex : texLayer) {
			tex.move(change);
		}
	}
}

void nv::Sprite::setPos(SDL_Point p) noexcept {
	setPos(p.x, p.y);
}

void nv::Sprite::move(int x, int y) noexcept {
	for (auto& texData : texObjLayers.at(currLayer)) {
		texData.move(x, y);
	}
}

void nv::Sprite::move(SDL_Point p) noexcept {
	move(p.x, p.y);
}

void nv::Sprite::scale(int x, int y) noexcept {
	for (auto& texData : texObjLayers.at(currLayer)) {
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
	return ranges::any_of(texObjLayers.at(currLayer), [&](const auto& texData) { 
		return texData.containsCoord(x, y);
	});
}

bool nv::Sprite::containsCoord(SDL_Point p) const noexcept {
	return containsCoord(p.x, p.y);
}

void nv::Sprite::setOpacity(Uint8 opacity) {
	for (auto& texLayer : texObjLayers) {
		for (auto& tex : texLayer) {
			tex.setOpacity(opacity);
		}
	}
}

void nv::Sprite::render(SDL_Renderer* renderer) const noexcept {
	for (const auto& texData : texObjLayers.at(currLayer)) {
		texData.render(renderer);
	}
}
