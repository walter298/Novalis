#pragma once

#include "data_util/BasicConcepts.h"

#include <SDL3/SDL_render.h>
#include <SDL3/SDL_rect.h>

#include "ID.h"

namespace nv {
	struct Rect {
		static bool isInRegion(float mx, float my, float x, float y, float w, float h) noexcept;
		static bool isInRegion(SDL_FPoint coord, float x, float y, float w, float h) noexcept;

		SDL_FRect rect{ 0.0f, 0.0f, 0.0f, 0.0f };
		float currScale = 1.0f;

		Rect() noexcept = default;
		inline Rect(SDL_FRect rect) noexcept : rect{ rect } 
		{
		}

		inline void scale(float newScale, SDL_FPoint refPoint = { 0.0f, 0.0f }) noexcept {
			auto scaleFactor = (newScale / currScale);
			rect.x = rect.x + scaleFactor * (refPoint.x - rect.x);
			rect.y = rect.y + scaleFactor * (refPoint.y - rect.y);
			rect.w *= scaleFactor;
			rect.h *= scaleFactor;
			currScale = newScale;
		}

		bool containsCoord(SDL_FPoint p) const noexcept;
		void move(float dx, float dy) noexcept;
		void move(SDL_FPoint p) noexcept;
		void setPos(float x, float y) noexcept;
		void setPos(SDL_FPoint p) noexcept;
		SDL_FPoint getPos() const noexcept;
		void setSize(float w, float h) noexcept;
		void setSize(SDL_FPoint p) noexcept;
		SDL_FPoint getSize() const noexcept;
		
		void save(json& json) const;

		friend void to_json(json& j, const Rect& r);
		friend void from_json(const json& j, Rect& r);
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
		uint8_t r = 0, g = 0, b = 0, a = 0;
		SDL_GetRenderDrawColor(renderer, &r, &g, &b, &a);
		SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
		SDL_RenderFillRect(renderer, &rect);
		SDL_SetRenderDrawColor(renderer, r, g, b, a);
	}
	inline void renderSDLRect(SDL_Renderer* renderer, SDL_Rect rect, SDL_Color color) {
		renderSDLRect(renderer, toSDLFRect(rect), color);
	}

	void to_json(json& j, const Rect& r);
	void from_json(const json& j, Rect& r);
}