#pragma once

#include <filesystem>
#include <memory>
#include <print>
#include <string_view>
#include <variant>
#include <boost/unordered/unordered_flat_map.hpp>
#include <SDL3_image/SDL_image.h>
#include <SDL3/SDL_render.h>

#include "Rect.h"

namespace nv {
	struct TexturePtr {
		SDL_Texture* tex = nullptr;

		TexturePtr() = default;
		inline TexturePtr(SDL_Texture* tex) : tex{ tex }
		{
		}
		inline TexturePtr(SDL_Renderer* renderer, const char* path) : tex{ IMG_LoadTexture(renderer, path) }  
		{
			if (tex == nullptr) {
				std::println("Error: could not load texture {}", path);
				std::println("{}", SDL_GetError());
				exit(-1);
			}
		}
		inline TexturePtr(const TexturePtr& other) noexcept : tex{ other.tex } {
			if (tex != nullptr) {
				tex->refcount++;
			}
		}
		inline ~TexturePtr() noexcept {
			SDL_DestroyTexture(tex); //will decrement refcount and then only destroy if the refcount is 0
		}
	};

	using TextureMap = boost::unordered_flat_map<std::string, TexturePtr>;

	class Texture {
	protected:
		void setTextureDimensions() {
			float w = 0.0f, h = 0.0f;
			SDL_GetTextureSize(tex.tex, &w, &h);
			ren.setSize(w, h);
		}
		void renderImpl(SDL_Renderer* renderer, SDL_Texture* tex) const noexcept {
			SDL_RenderTextureRotated(renderer, tex, nullptr, &ren.rect, angle, &rotationPoint, flip);
		}
	public:
		TexturePtr tex;
		Rect ren;
		SDL_FPoint rotationPoint{ 0, 0 };
		double angle = 0.0;
		SDL_FlipMode flip = SDL_FLIP_NONE;
		
		inline Texture(TexturePtr tex) noexcept : tex{ tex } {
			float w = 0.0f, h = 0.0f;
			SDL_GetTextureSize(tex.tex, &w, &h);
			ren.setSize(w, h);
		}

		inline Texture(SDL_Renderer* renderer, const char* path) noexcept : Texture{ TexturePtr{ renderer, path } } 
		{
		}

		inline void render(SDL_Renderer* renderer) const noexcept {
			SDL_RenderTextureRotated(renderer, tex.tex, nullptr, &ren.rect, angle, &rotationPoint, flip);
		}

		inline void screenScale(float newScale, SDL_FPoint refPoint) noexcept {
			ren.scale(newScale, refPoint);
		}
		inline void screenMove(SDL_FPoint change) noexcept {
			ren.move(change);
		}
		inline SDL_FPoint getScreenPos() const noexcept {
			return ren.getPos();
		}
		inline SDL_FPoint getScreenSize() const noexcept {
			return ren.getSize();
		}
		inline void setScreenSize(SDL_FPoint p) noexcept {
			ren.setSize(p);
		}
		inline void rotate(double angle, SDL_FPoint rotationPoint) noexcept {
			this->angle = angle;
			this->rotationPoint = rotationPoint;
		}
		inline void setOpacity(uint8_t opacity) noexcept {
			SDL_SetTextureAlphaMod(tex.tex, opacity);
		}
	};

	using TextureRef = std::reference_wrapper<Texture>;
}
