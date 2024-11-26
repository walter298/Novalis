#include "PolygonEditor.h"

void nv::editor::PolygonBuilder::edit() {
	
}

nv::editor::PolygonEditor::PolygonEditor(std::span<SDL_Point> points) {
	for (const auto& [x, y] : points) {
		polygon.outer().emplace_back(x, y);
	}
}

nv::editor::PolygonEditor::PolygonEditor(SDL_Renderer* renderer, std::span<SDL_Point> points)
	: m_renderer{ renderer } {
}

bool nv::editor::PolygonEditor::select() {
	return false;
}

void nv::editor::PolygonEditor::render() const {
	SDL_SetRenderDrawColor(m_renderer, 0, 255, 0, 255);
	auto points = std::views::zip(polygon.outer(), polygon.outer() | std::views::drop(1));
	for (const auto& [p1, p2] : points) {
		SDL_RenderDrawLine(m_renderer, p1.get<0>(), p1.get<1>(), p2.get<0>(), p2.get<1>());
	}
	auto first = polygon.outer().front();
	auto last = polygon.outer().back();
	SDL_RenderDrawLine(m_renderer, first.get<0>(), first.get<1>(), last.get<0>(), last.get<1>());
}

void nv::editor::PolygonEditor::save(json& json) const {
	json = polygon;
}
