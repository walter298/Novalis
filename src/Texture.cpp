#include "Texture.h"

#include <print>

nv::Texture::Texture(SDL_Texture* tex) noexcept
	: raw(tex) {}

nv::Texture::~Texture() noexcept {
	SDL_DestroyTexture(raw);
}

nv::TextureObject::TextureObject(TexturePtr tex, TextureData texData) 
	: tex{ std::move(tex) }, texData{ std::move(texData) } 
{
}

nv::TextureObject::TextureObject(std::string_view jsonPath, SDL_Renderer* renderer) {
	std::ifstream jsonFile{ jsonPath.data() };
	assert(jsonFile.is_open());
	auto json = json::parse(jsonFile);
	texData = json.get<TextureData>();
	tex = std::make_shared<Texture>(IMG_LoadTexture(renderer, json["texture_path"].get<std::string>().c_str()));
}

void nv::TextureObject::setOpacity(Uint8 opacity) noexcept {
	SDL_SetTextureAlphaMod(tex->raw, opacity);
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
	SDL_RenderDrawPoint(renderer, texData.rotationPoint.x, texData.rotationPoint.y);
	SDL_RenderCopyEx(renderer, tex->raw, nullptr, &texData.ren.rect, texData.angle, &texData.rotationPoint, texData.flip);
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
