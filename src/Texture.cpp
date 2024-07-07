#include "Texture.h"

#include <print>

nv::TextureRAII::TextureRAII(SDL_Texture* tex) noexcept
	: raw(tex) 
{
}

nv::TextureRAII::~TextureRAII() noexcept {
	SDL_DestroyTexture(raw);
}

nv::TextureObject::TextureObject(std::string_view path, TexturePtr texPtr, TextureData texData) 
	: texPath{ std::make_shared<std::string>(path) }, m_texVariant{ std::move(texPtr) }, texData{ std::move(texData) }
{
	tex = std::get<TexturePtr>(m_texVariant)->raw;
}

nv::TextureObject::TextureObject(std::string_view texPath, SDL_Texture* rawTex, TextureData texData)
	: texPath{ std::make_shared<std::string>(texPath) }, m_texVariant { std::move(rawTex) }, texData{ std::move(texData) }
{
	tex = rawTex;
}

nv::TextureObject::TextureObject(SDL_Renderer* renderer, const json& json, TextureMap& texMap) {
	auto texPath = json["texture_path"].get<std::string>();
	auto texPathIt = texMap.find(texPath);
	if (texPathIt != texMap.end()) {
		tex = texPathIt->second.raw;
	} else {
		tex = IMG_LoadTexture(renderer, texPath.c_str());
		texMap.emplace(std::piecewise_construct, std::forward_as_tuple(std::move(texPath)), std::forward_as_tuple(tex));
	}
	texData = json["texture_object_data"].get<TextureData>();
}

void nv::TextureObject::setOpacity(Uint8 opacity) noexcept {
	SDL_SetTextureAlphaMod(tex, opacity);
}

void nv::TextureObject::setPos(int x, int y) noexcept {
	texData.ren.setPos(x, y);
	texData.world.setPos(x, y);
}

void nv::TextureObject::setPos(SDL_Point pos) noexcept {
	setPos(pos.x, pos.y);
}

SDL_Point nv::TextureObject::getPos() const noexcept {
	return SDL_Point{ texData.ren.rect.x, texData.ren.rect.y };
}

void nv::TextureObject::move(int dx, int dy) noexcept {
	texData.ren.move(dx, dy);
	texData.world.move(dx, dy);
}

void nv::TextureObject::move(SDL_Point change) noexcept {
	move(change.x, change.y);
}

void nv::TextureObject::setSize(int w, int h) noexcept {
	texData.ren.setSize(w, h);
	texData.world.setSize(w, h);
}

void nv::TextureObject::setSize(SDL_Point p) {
	setSize(p.x, p.y);
}

void nv::TextureObject::scale(int dx, int dy) noexcept {
	texData.ren.scale(dx, dy);
	texData.world.scale(dx, dy);
}

void nv::TextureObject::scale(SDL_Point change) noexcept {
	scale(change.x, change.y);
}

void nv::TextureObject::rotate(double angle, SDL_Point rotationPoint) noexcept {
	texData.angle = angle;
	texData.rotationPoint = rotationPoint;
}

void nv::TextureObject::render(SDL_Renderer* renderer) const noexcept {
	SDL_RenderCopyEx(renderer, tex, nullptr, &texData.ren.rect, texData.angle, &texData.rotationPoint, texData.flip);
}

void nv::TextureObject::save(json& json) const {
	json["texture_path"] = *texPath;
	json["texture_object_data"] = texData;
}

void nv::TextureObject::setRotationCenter() noexcept {
	texData.rotationPoint = {
		(texData.ren.rect.w) / 2,
		(texData.ren.rect.h) / 2
	};
}

bool nv::TextureObject::containsCoord(int x, int y) const noexcept {
	return texData.ren.containsCoord(x, y);
}

bool nv::TextureObject::containsCoord(SDL_Point p) const noexcept {
	return containsCoord(p.x, p.y);
}
