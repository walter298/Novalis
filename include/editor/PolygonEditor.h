#pragma once

#include <SDL2/SDL.h>

#include "../Collision.h"

namespace nv {
	namespace editor {
		struct PolygonBuilder {
			std::vector<SDL_Point> points;

			void edit();
		};

		class PolygonEditor {
		private:
			SDL_Renderer* m_renderer = nullptr;
		public:
			BGPolygon polygon;
			
			PolygonEditor() = default;
			PolygonEditor(std::span<SDL_Point> points);
			PolygonEditor(SDL_Renderer*, std::span<SDL_Point> points);

			bool select();
			void render() const;
			void save(json& json) const;
		};
	}
}
