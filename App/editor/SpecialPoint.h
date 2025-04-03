#pragma once

#include <string>
#include <SDL3/SDL_render.h>

#include "../Rect.h"

namespace nv {
	namespace editor {
		struct SpecialPoint {
			static constexpr float WIDTH = 15.0f;
			static constexpr SDL_Color COLOR{ 0, 255, 0, 255 };

			std::string name;
			Point point{};

			void render(SDL_Renderer* renderer) const noexcept {
				SDL_FRect rect{ point.x, point.y, WIDTH, WIDTH };
				renderSDLRect(renderer, rect, COLOR);
			}

			bool containsCoord(Point p) const noexcept {
				SDL_FPoint sdlP = p;
				SDL_FRect rect{ point.x, point.y, WIDTH, WIDTH };
				return SDL_PointInRectFloat(&sdlP, &rect);
			}
		};
	}
}