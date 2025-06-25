#include "Polygon.h"

void nv::detail::renderScreenPoints(SDL_Renderer* renderer, uint8_t opacity, const std::span<const Point>& points, SDL_Color color)
{
	SDL_Color originalDrawColor;
	SDL_GetRenderDrawColor(renderer, &originalDrawColor.r, &originalDrawColor.g, &originalDrawColor.b, &originalDrawColor.a);

	SDL_BlendMode originalBlendMode;
	SDL_GetRenderDrawBlendMode(renderer, &originalBlendMode);

	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_ADD);
	SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, opacity);

	auto pointPairs = std::views::zip(points, points | std::views::drop(1));
	for (const auto& [p1, p2] : pointPairs) {
		SDL_RenderLine(renderer, p1.x, p1.y, p2.x, p2.y);
	}

	SDL_SetRenderDrawColor(renderer, originalDrawColor.r, originalDrawColor.g, originalDrawColor.b, originalDrawColor.a);
	SDL_SetRenderDrawBlendMode(renderer, originalBlendMode);
}
