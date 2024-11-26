#pragma once

#include <SDL2/SDL_render.h>

namespace nv {
	namespace editor {
		class SpecialPoint {
		private:
			SDL_Renderer* m_renderer = nullptr;
		public:
			static constexpr int RADIUS = 12;

			SDL_Point point{};

			SpecialPoint(SDL_Renderer* renderer) noexcept;
			void render() const noexcept;
			bool containsCoord(int x, int y) const noexcept;
			bool containsCoord(SDL_Point p) const noexcept;
		};
	}
}

