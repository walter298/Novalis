#pragma once

#include <concepts>
#include <filesystem>
#include <mdspan>
#include <memory>
#include <optional>
#include <print>
#include <span>
#include <string_view>
#include <variant>
#include <boost/unordered/unordered_flat_map.hpp>
#include <SDL3_image/SDL_image.h>
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_surface.h>
#include "detail/reflection/ClassIteration.h"
#include "Rect.h"

namespace nv {
	struct TexturePtr {
		SDL_Texture* tex = nullptr;

		TexturePtr() = default;
		inline TexturePtr(SDL_Texture* tex) : tex{ tex }
		{
		}
		inline TexturePtr(SDL_Renderer* renderer, const char* path) {
			tex = IMG_LoadTexture(renderer, path);
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

		MAKE_INTROSPECTION(tex);
	};

	inline double getBrightness(const SDL_Color& c) {
		return 0.2126 * static_cast<double>(c.r) +
			0.7152 * static_cast<double>(c.g) +
			0.0722 * static_cast<double>(c.b);
	}

	class PixelData {
	private:
		SDL_Surface* m_surface = nullptr;
	public:
		struct Pixel {
			int x = 0;
			int y = 0;
		};
		PixelData(SDL_Renderer* renderer, SDL_Texture* tex, const SDL_Rect& rect) {
			auto frect = toSDLFRect(rect);

			SDL_RenderClear(renderer);
			SDL_RenderTexture(renderer, tex, nullptr, &frect);
			m_surface = SDL_RenderReadPixels(renderer, nullptr);
			if (!m_surface) {
				std::println("{}", SDL_GetError());
				std::abort();
			}
		}
		PixelData(const PixelData&) noexcept = delete;
		PixelData(PixelData&&) noexcept = default;
		~PixelData() noexcept {
			SDL_DestroySurface(m_surface);
		}

		void setColor(int x, int y, SDL_Color color) noexcept {
			auto res = SDL_WriteSurfacePixel(m_surface, x, y, color.r, color.g, color.b, color.a);
			assert(res);
		}

		SDL_Color getColor(int x, int y) const noexcept {
			SDL_Color ret;
			bool successfullyReadPixelData = SDL_ReadSurfacePixel(m_surface, x, y, &ret.r, &ret.g, &ret.b, &ret.a);
			assert(successfullyReadPixelData);
			return ret;
		}
		int getWidth() const noexcept {
			return m_surface->w;
		}
		int getHeight() const noexcept {
			return m_surface->h;
		}

		template<typename Func>
		void forEachSubRectangle(int w, int h, Func f) {
			assert(w >= 0 && w <= getWidth() && h >= 0 && h <= getHeight());

			std::vector<Pixel> pixelStorage(static_cast<size_t>(w * h));

			using Subrectangle = std::mdspan<Pixel, std::dextents<int, 2>>;
			Subrectangle rectangle{ pixelStorage.data(), h, w }; //IMPORTANT: rows come before columns

			auto fillRectangle = [&](int x, int y) {
				for (int i = 0; i < h; i++) {
					for (int j = 0; j < w; j++) {
						rectangle[i, j] = { x + j, y + i };
					}
				}
			};

			for (int y = 0; y + h <= getHeight(); y++) {
				for (int x = 0; x + w <= getWidth(); x++) {
					fillRectangle(x, y);
					f(rectangle);
				}
			}
		}

		template<std::invocable<const SDL_Color&> Pred>
		std::optional<Pixel> find(Pred pred) const noexcept {
			for (int x = 0; x < getWidth(); x++) {
				for (int y = 0; y < getHeight(); y++) {
					auto color = getColor(x, y);
					if (pred(color)) {
						return Pixel{ x, y };
					}
				}
			}
			return std::nullopt;
		}
	};
	
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

		void screenScale(float newScale, SDL_FPoint refPoint = { 0, 0 }) noexcept {
			texData.ren.scale(newScale);
		}
		void worldScale(float newScale, SDL_FPoint refPoint = { 0, 0 }) noexcept {
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

		PixelData calcPixelData(SDL_Renderer* renderer) const {
			return { renderer, tex.tex, toSDLRect(texData.ren.sdlRect()) };
		}
		
		MAKE_INTROSPECTION(tex, texData);
	};

	using TextureRef = std::reference_wrapper<Texture>;
}
