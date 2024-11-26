#include "PolygonBuilder.h"

#include <imgui_impl_sdl2.h>
#include <imgui_impl_sdlrenderer2.h>

void nv::editor::PolygonBuilder::renderLinkedPoints() const noexcept {

}

nv::editor::SpecialPoint& nv::editor::PolygonBuilder::renderSpecialPoint() {
	if (m_placingNewPoint) {
		return m_linkedPoints.back();
	} else {
		m_placingNewPoint = true;
		return m_linkedPoints.emplace_back(m_renderer);
	}
}

nv::BGPolygon nv::editor::PolygonBuilder::makePolygon() {
	BGPolygon ret;
	ret.outer().append_range(m_linkedPoints | std::views::transform([](auto& renderPoint) {
		return BGPoint{ renderPoint.point.x, renderPoint.point.y };
	}));
	return ret;
}

nv::editor::PolygonBuilder::PolygonBuilder(SDL_Renderer* renderer) noexcept
	: m_renderer{ renderer }
{
}

std::optional<nv::BGPolygon> nv::editor::PolygonBuilder::operator()(SDL_Point mouse) noexcept {
	ImGui::SetMouseCursor(ImGuiMouseCursor_None);
	SpecialPoint& point = [this]() -> SpecialPoint& {
		if (m_placingNewPoint) {
			return m_linkedPoints.back();
		} else {
			m_placingNewPoint = true;
			return m_linkedPoints.emplace_back(m_renderer);
		}
	}();

	auto& movedPoint = m_placingNewPoint ? m_linkedPoints.back() : m_linkedPoints.emplace_back(m_renderer);
	movedPoint.point = mouse;
	point.render();

	if (ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
		if (m_linkedPoints.front().containsCoord(mouse)) {
			auto ret = m_polygon;
			m_polygon.clear();
			auto polygon = makePolygon();
			m_linkedPoints.clear();
			building = false;
			return polygon;
		} else {
			m_placingNewPoint = false;
			return std::nullopt;
		}
	}
}