#pragma once

#include "detail/memory/TexturePtr.h"
#include "detail/TextureData.h"

namespace nv {

	struct Texture : public detail::TextureRenderData {
		detail::TexturePtr tex;
		
		Texture() noexcept = default;

		Texture(detail::TexturePtr tex, detail::TextureRenderData data) 
			: tex{ tex }, TextureRenderData{ data }
		{
		}

		Texture(detail::TexturePtr tex) noexcept : tex{ tex } {
			float w = 0.0f, h = 0.0f;
			SDL_GetTextureSize(tex.tex, &w, &h);
			ren.setSize(w, h);
			world.setSize(w, h);
		}

		Texture(SDL_Renderer* renderer, const char* path) noexcept : Texture{ { renderer, path } } 
		{
		}

		void render(SDL_Renderer* renderer) const noexcept {
			auto [angle, rotationPoint] = screenRotationData;
			SDL_FPoint rotationPointC = rotationPoint;
			auto rect = ren.sdlRect();
			auto angleD = static_cast<double>(screenRotationData.angle);
			SDL_RenderTextureRotated(renderer, tex.tex, nullptr, &rect, angleD, &rotationPointC, flip);
		}

		void setOpacity(uint8_t opacity) noexcept {
			SDL_SetTextureAlphaMod(tex.tex, opacity);
		}
		
		MAKE_INTROSPECTION(tex, ren, world, screenRotationData, worldRotationData, flip);
	};

	using TextureRef = std::reference_wrapper<Texture>;
}
