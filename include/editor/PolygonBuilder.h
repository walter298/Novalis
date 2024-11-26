#pragma once

#include <optional>
#include <vector>

#include "../Collision.h"
#include "SpecialPoint.h"

namespace nv {
	namespace editor {
		class PolygonBuilder {
		private:
			SDL_Renderer* m_renderer;
			std::vector<SpecialPoint> m_linkedPoints;
			bool m_placingNewPoint = false;

			void renderLinkedPoints() const noexcept;
			SpecialPoint& renderSpecialPoint();

			BGPolygon makePolygon();
		public:
			bool building = false;

			PolygonBuilder(SDL_Renderer* m_renderer) noexcept;

			std::optional<BGPolygon> operator()(SDL_Point mouse) noexcept;
		};
	}
}

