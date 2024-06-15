#pragma once

#include <fstream>
#include <string_view>

#include <SDL2/SDL_image.h>

#include "Rect.h"

namespace nv {
	struct Texture {
		SDL_Texture* raw = nullptr;

		Texture() = default;
		explicit Texture(SDL_Texture* texture) noexcept;

		Texture(const Texture&) = delete;
		Texture& operator=(const Texture&) = delete;

		Texture(Texture&&) noexcept = default;
		Texture& operator=(Texture&&) noexcept = default;

		~Texture() noexcept;
	};

	using TexturePtr = std::shared_ptr<Texture>;

	struct TextureData {
		Rect ren;
		Rect world;
		SDL_Point rotationPoint{ 0, 0 };
		double angle = 0.0;
		SDL_RendererFlip flip = SDL_FLIP_NONE;
	};

	struct TextureObject {
		TextureObject() = default;
		TextureObject(TexturePtr texPtr, TextureData texData);
		TextureObject(std::string_view jsonPath, SDL_Renderer* renderer);

		TexturePtr tex;
		TextureData texData;
		
		void setOpacity(Uint8 opacity) noexcept;

		void setPos(int x, int y) noexcept;
		void setPos(SDL_Point pos) noexcept;

		SDL_Point getPos() const noexcept;

		void move(int dx, int dy) noexcept;
		void move(SDL_Point change) noexcept;

		void scale(int dx, int dy) noexcept;
		void scale(SDL_Point change) noexcept;

		void rotate(double angle, SDL_Point rotationPoint) noexcept;
		void setRotationCenter() noexcept;

		bool containsCoord(int x, int y) const noexcept;
		bool containsCoord(SDL_Point p) const noexcept;

		void render(SDL_Renderer* renderer) const noexcept;
	};

	/*void from_json(const json& j, TexturePos& pos);
	void to_json(json& j, const TexturePos& pos);*/
}