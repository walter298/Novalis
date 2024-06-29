#pragma once

#include <fstream>
#include <string_view>

#include <SDL2/SDL_image.h>

#include "Rect.h"

namespace nv {
	struct TextureDestructorWrapper {
		SDL_Texture* raw = nullptr;

		TextureDestructorWrapper() = default;
		explicit TextureDestructorWrapper(SDL_Texture* texture) noexcept;

		TextureDestructorWrapper(const TextureDestructorWrapper&) = delete;
		TextureDestructorWrapper& operator=(const TextureDestructorWrapper&) = delete;

		TextureDestructorWrapper(TextureDestructorWrapper&&) noexcept = default;
		TextureDestructorWrapper& operator=(TextureDestructorWrapper&&) noexcept = default;

		~TextureDestructorWrapper() noexcept;
	};

	using TexturePtr = std::shared_ptr<TextureDestructorWrapper>;

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
		
		TexturePtr tex;
		TextureData texData;
		
		void setOpacity(Uint8 opacity) noexcept;

		void setPos(int x, int y) noexcept;
		void setPos(SDL_Point pos) noexcept;

		SDL_Point getPos() const noexcept;

		void move(int dx, int dy) noexcept;
		void move(SDL_Point change) noexcept;

		void setSize(int w, int h) noexcept;
		void setSize(SDL_Point p);

		void scale(int dx, int dy) noexcept;
		void scale(SDL_Point change) noexcept;

		void rotate(double angle, SDL_Point rotationPoint) noexcept;
		void setRotationCenter() noexcept;

		bool containsCoord(int x, int y) const noexcept;
		bool containsCoord(SDL_Point p) const noexcept;

		void render(SDL_Renderer* renderer) const noexcept;
	};
}