#include <optional>
#include <ranges>
#include <vector>
#include <novalis/Point.h>
#include <novalis/Polygon.h>

#include "imgui/imgui.h"
#include "SpecialPoint.h"

namespace nv {
	namespace editor {
		class PolygonBuilder {
		private:
			std::vector<Point> m_screenPoints;
			bool m_placingNewPoint = false;

			SpecialPoint m_firstPoint;
			SpecialPoint m_lastPlacedPoint;
			bool m_building = false;

			DynamicPolygon createPolygon(float worldOffsetX, float worldOffsetY) {
				//make the first point of the polygon also be the last so that the shape forms a closed ring
				auto first = m_screenPoints.front();
				m_screenPoints.push_back(std::move(first));

				auto makeWorldPoints = m_screenPoints | std::views::transform([&](const Point& point) {
					return Point{ point.x + worldOffsetX, point.y + worldOffsetY };
					});
				std::vector<Point> worldPoints;
				worldPoints.append_range(makeWorldPoints);

				DynamicPolygon poly{ std::move(m_screenPoints), std::move(worldPoints) };
				m_screenPoints.clear();
				m_building = false;
				return poly;
			}
		public:
			bool building() const noexcept {
				return m_building;
			}

			std::optional<DynamicPolygon> operator()(SDL_Renderer* renderer, Point mouse, float worldOffsetX, float worldOffsetY) noexcept {
				if (!m_building) {
					m_screenPoints.clear();
				}

				ImGui::SetMouseCursor(ImGuiMouseCursor_None);
				m_lastPlacedPoint.point = mouse;
				m_lastPlacedPoint.render(renderer);

				if (!m_screenPoints.empty()) {
					m_firstPoint.point = m_screenPoints.front();
					m_firstPoint.render(renderer);
					nv::detail::renderScreenPoints(renderer, 255, m_screenPoints);
				}

				if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
					m_building = true;
					if (m_firstPoint.containsCoord(mouse) && m_screenPoints.size() > 1) {
						return createPolygon(worldOffsetX, worldOffsetY);
					}
					else {
						m_screenPoints.emplace_back(mouse.x, mouse.y);
						m_lastPlacedPoint.point = mouse;
					}
				}
				return std::nullopt;
			}
		};
	}
}