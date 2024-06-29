#pragma once

#include <fstream>
#include <string_view>

#include <SDL2/SDL_image.h>

#include "DataUtil.h"
#include "Texture.h"

namespace nv {
	struct Sprite {
		using JsonFormat = Layers<std::pair<std::string, TextureData>>;

		using TextureLayers = Layers<TextureObject>;
		TextureLayers texObjLayers;
		int currLayer = 0;

		TextureData& texData(size_t texIdx);

		void setPos(int destX, int destY) noexcept;
		void setPos(SDL_Point p) noexcept;

		void move(int x, int y) noexcept;
		void move(SDL_Point p) noexcept;

		void scale(int x, int y) noexcept;
		void scale(SDL_Point p) noexcept;

		void rotate(double angle, SDL_Point p);
		void setRotationCenter() noexcept;

		bool containsCoord(int x, int y) const noexcept;
		bool containsCoord(SDL_Point p) const noexcept;

		void setOpacity(Uint8 opacity);

		void render(SDL_Renderer* renderer) const noexcept;
	};

	using Sprites = std::vector<Sprite>;
}