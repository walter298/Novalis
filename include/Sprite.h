#pragma once

#include <fstream>
#include <string_view>

#include <SDL2/SDL_image.h>

#include "DataUtil.h"
#include "Texture.h"

namespace nv {
	struct Sprite {
		static constexpr std::string_view textureRenPairsJkey = "textures_and_rens";
		static constexpr std::string_view textureCountJkey = "texture_size";
		
		using TextureLayers = std::vector<std::vector<TextureObject>>;
		TextureLayers texObjLayers;
		int currLayer = 0;

		using JsonFormat = std::vector<std::vector<std::pair<std::string, TextureData>>>;

		Sprite() = default;
		Sprite(std::string_view path, SDL_Renderer* renderer) noexcept;
		
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