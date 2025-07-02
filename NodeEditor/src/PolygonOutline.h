#pragma once

#include <vector>
#include <novalis/Polygon.h>
#include <novalis/Texture.h>

namespace nv {
	namespace editor {
		std::vector<DynamicPolygon> getPolygonOutlines(SDL_Renderer* renderer, Texture& tex) noexcept;
	}
}