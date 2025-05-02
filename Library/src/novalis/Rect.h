#pragma once

#include "detail/reflection/ClassIteration.h"

#include <SDL3/SDL_render.h>
#include <SDL3/SDL_rect.h>

#include "ID.h"
#include "Point.h"

namespace nv {
	struct Rect {
		static bool isInRegion(float mx, float my, float x, float y, float w, float h) noexcept;
		static bool isInRegion(Point coord, float x, float y, float w, float h) noexcept;

		Point pos;
		Point size;
		
		float currScale = 1.0f;

		Rect() noexcept : pos{ 0.0f, 0.0f }, size{ 0.0f, 0.0f } {}
		Rect(SDL_FRect rect) noexcept : pos{ rect.x, rect.y }, size{ rect.w, rect.h }
		{
		}

		inline void scale(float newScale, Point refPoint = { 0.0f, 0.0f }) noexcept {
			auto scaleFactor = (newScale / currScale);
			pos.x = pos.x + scaleFactor * (refPoint.x - pos.x);
			pos.y = pos.y + scaleFactor * (refPoint.y - pos.y);
			size.x *= scaleFactor;
			size.y *= scaleFactor;
			currScale = newScale;
		}

		bool containsCoord(Point p) const noexcept;
		Point move(float dx, float dy) noexcept;
		Point move(Point p) noexcept;
		Point setPos(float x, float y) noexcept;
		Point setPos(Point p) noexcept;
		Point getPos() const noexcept;
		Point setSize(float w, float h) noexcept;
		Point setSize(Point p) noexcept;
		Point getSize() const noexcept;
		
		SDL_FRect sdlRect() const {
			return { pos.x, pos.y, size.x, size.y };
		}

		MAKE_INTROSPECTION(pos, size, currScale)
	};

	inline SDL_Rect toSDLRect(SDL_FRect frect) {
		return {
			static_cast<int>(frect.x), static_cast<int>(frect.y),
			static_cast<int>(frect.w), static_cast<int>(frect.h)
		};
	}

	inline SDL_FRect toSDLFRect(SDL_Rect r) {
		return {
			static_cast<float>(r.x), static_cast<float>(r.y),
			static_cast<float>(r.w), static_cast<float>(r.h)
		};
	}

	inline void renderSDLRect(SDL_Renderer* renderer, SDL_FRect rect, SDL_Color color) {
		SDL_Color originalDrawColor;
		SDL_GetRenderDrawColor(renderer, &originalDrawColor.r, &originalDrawColor.g, &originalDrawColor.b, &originalDrawColor.a);

		SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
		SDL_RenderFillRect(renderer, &rect);

		SDL_SetRenderDrawColor(renderer, originalDrawColor.r, originalDrawColor.g, originalDrawColor.b, originalDrawColor.a);
	}

	inline void renderSDLRect(SDL_Renderer* renderer, SDL_Rect rect, SDL_Color color) {
		renderSDLRect(renderer, toSDLFRect(rect), color);
	}
}