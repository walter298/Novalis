#include "Collision.h"

void nv::Polygon::move(SDL_FPoint p) noexcept {
	for (BGPoint& point : rep.outer()) {
		point.set<0>(point.get<0>() + p.x);
		point.set<1>(point.get<1>() + p.y);
	}
}

bool nv::Polygon::containsCoord(SDL_FPoint p) const noexcept {
	return bg::intersects(rep, BGPoint{ p.x, p.y });
}

void nv::Polygon::save(json& j) const {
	j = rep;
}

SDL_FPoint nv::toSDLFPoint(BGPoint p) noexcept {
	return { p.get<0>(), p.get<1>() };
}
