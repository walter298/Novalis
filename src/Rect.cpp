#include "Rect.h"

#include "data_util/BasicJsonSerialization.h"

bool nv::Rect::isInRegion(float mx, float my, float x, float y, float w, float h) noexcept {
	return mx > x && mx < x + w &&
		my > y && my < y + h;
}

bool nv::Rect::isInRegion(SDL_FPoint coord, float x, float y, float w, float h) noexcept {
	return isInRegion(coord.x, coord.y, x, y, w, h);
}

bool nv::Rect::containsCoord(SDL_FPoint p) const noexcept {
	return isInRegion(p, rect.x, rect.y, rect.w, rect.h);
}

void nv::Rect::move(float dx, float dy) noexcept {
	rect.x += dx;
	rect.y += dy;
}
void nv::Rect::move(SDL_FPoint p) noexcept {
	move(p.x, p.y);
}
void nv::Rect::setPos(float x, float y) noexcept {
	rect.x = x;
	rect.y = y;
}
void nv::Rect::setPos(SDL_FPoint p) noexcept {
	setPos(p.x, p.y);
}
SDL_FPoint nv::Rect::getPos() const noexcept {
	return SDL_FPoint{ rect.x, rect.y };
}
void nv::Rect::setSize(float w, float h) noexcept {
	rect.w = w;
	rect.h = h;
}
void nv::Rect::setSize(SDL_FPoint p) noexcept
{
	setSize(p.x, p.y);
}

SDL_FPoint nv::Rect::getSize() const noexcept {
	return { rect.w, rect.h };
}

void nv::Rect::save(json& json) const {
	json = *this;
}

void nv::to_json(json& j, const Rect& r) {
	j["sdl_rect"] = r.rect;
}

void nv::from_json(const json& j, Rect& r) {
	r.rect = j.at("sdl_rect").get<SDL_FRect>();
}