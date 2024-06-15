#pragma once

#include <fstream>
#include <string_view>
#include <vector>

#include <SDL2/SDL_image.h>

#include "DataUtil.h"
#include "ID.h"
#include "Texture.h"

namespace nv {
	namespace editor {
		class SpriteEditor;
	}

	class Sprite {
	private:
		static constexpr std::string_view textureRenPairsJkey = "textures_and_rens";
		static constexpr std::string_view textureCountJkey = "texture_size";
		
		Layers<TextureObject, std::vector> m_textureLayers;
		int m_currLayer = 0;

		Sprite() = default;
	public:
		using JsonFormat = std::unordered_map<std::string, std::pair<int, std::vector<TextureData>>>;

		template<std::invocable<std::string_view, int, TexturePtr, TextureData> InsertFunc>
		static void loadTextureData(std::string_view jsonPath, SDL_Renderer* renderer, InsertFunc insert) {
			std::ifstream file{ jsonPath.data() };
			auto json = json::parse(file);
			auto texMap = json.get<JsonFormat>();
			for (auto& [texPath, texData] : texMap) {
				auto& [layer, texDataElems] = texData;
				auto tex = std::make_shared<Texture>(IMG_LoadTexture(renderer, texPath.c_str()));
				for (auto& texDataElem : texDataElems) {
					insert(texPath, layer, tex, std::move(texDataElem));
				}
			}
		}

		Sprite(std::string_view path, SDL_Renderer* renderer) noexcept;

		void setLayer(int layer) noexcept; 

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

		void render(SDL_Renderer* renderer) const noexcept;

		friend class editor::SpriteEditor;
	};

	using Sprites = std::vector<Sprite>;
}