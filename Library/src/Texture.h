#pragma once

#include <filesystem>
#include <memory>
#include <print>
#include <string_view>
#include <variant>
#include <boost/describe.hpp>
#include <boost/unordered/unordered_flat_map.hpp>
#include <SDL3_image/SDL_image.h>
#include <SDL3/SDL_render.h>

#include "detail/reflection/ClassIteration.h"
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
			/*if (tex != nullptr) {
				tex->refcount++;
			}*/
		}
		inline ~TexturePtr() noexcept {
			//SDL_DestroyTexture(tex); //will decrement refcount and then only destroy if the refcount is 0
		}

		MAKE_INTROSPECTION(tex);
	};

	/*BOOST_DESCRIBE_STRUCT(TexturePtr, (), (tex));*/

	struct TextureRenderData {
		Rect ren;
		Rect world;
		Point rotationPoint{ 0, 0 };
		double angle = 0.0;
		SDL_FlipMode flip = SDL_FLIP_NONE;
	};

	struct Texture {
		TexturePtr tex;
		TextureRenderData texData;

		Texture() noexcept = default;

		Texture(TexturePtr tex, TextureRenderData texData) 
			: tex{ tex }, texData{ texData }
		{
		}

		Texture(TexturePtr tex) noexcept : tex{ tex } {
			float w = 0.0f, h = 0.0f;
			SDL_GetTextureSize(tex.tex, &w, &h);
			texData.ren.setSize(w, h);
		}

		Texture(SDL_Renderer* renderer, const char* path) noexcept : Texture{ { renderer, path } } 
		{
		}

		void render(SDL_Renderer* renderer) const noexcept {
			SDL_FPoint rotationPoint = texData.rotationPoint;
			auto rect = texData.ren.sdlRect();
			SDL_RenderTextureRotated(renderer, tex.tex, nullptr, &rect, texData.angle, &rotationPoint, texData.flip);
		}

		void screenScale(float newScale) noexcept {
			texData.ren.scale(newScale);
		}
		void worldScale(float newScale) noexcept {
			texData.world.scale(newScale);
		}
		float getScreenScale() const noexcept {
			return texData.ren.currScale;
		}
		float getWorldScale() const noexcept {
			return texData.world.currScale;
		}
		void screenMove(Point change) noexcept {
			texData.ren.move(change);
		}
		void worldMove(Point change) noexcept {
			texData.world.move(change);
		}
		Point getScreenPos() const noexcept {
			return texData.ren.getPos();
		}
		Point setScreenPos(Point p) noexcept {
			return texData.ren.setPos(p);
		}
		Point getWorldPos() const noexcept {
			return texData.world.getPos();
		}
		Point setWorldPos(Point p) noexcept {
			return texData.world.setPos(p);
		}
		Point getWorldSize() const noexcept {
			return texData.world.getSize();
		}
		Point setWorldSize(Point p) noexcept {
			return texData.world.setSize(p);
		}
		Point getScreenSize() const noexcept {
			return texData.ren.getSize();
		}
		Point setScreenSize(Point p) noexcept {
			return texData.ren.setSize(p);
		}
		void rotate(double angle, Point rotationPoint) noexcept {
			texData.angle = angle;
			texData.rotationPoint = rotationPoint;
		}
		void setOpacity(uint8_t opacity) noexcept {
			SDL_SetTextureAlphaMod(tex.tex, opacity);
		}
		bool containsScreenCoord(Point p) const noexcept {
			return texData.ren.containsCoord(p);
		}
		bool containsWorldCoord(Point p) const noexcept {
			return texData.world.containsCoord(p);
		}
		
		MAKE_INTROSPECTION(tex, texData);
	};

	/*BOOST_DESCRIBE_STRUCT(Texture, (), (tex, texData));
	NV_DEFINE_TYPE_NAME(Texture);*/

	using TextureRef = std::reference_wrapper<Texture>;
}
