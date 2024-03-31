#pragma once

#include <fstream>
#include <string_view>

#include <SDL2/SDL_image.h>

#include "ID.h"
#include "Namespace.h"
#include "Texture.h"

namespace nv {
	class MultitextureSprite : public IDObj {
	private:
		static constexpr std::string_view textureRenPairsJkey = "textures_and_rens";
		static constexpr std::string_view textureSizeJkey = "texture_size";
		static constexpr std::string_view layerIdxsJkey = "layer_idxs";

		using TextureLayers = std::shared_ptr<std::vector<std::pair<Texture, TexturePos>>>;
		TextureLayers m_textures;
		std::vector<size_t> m_layerIdxs;
		size_t m_currLayerIdx = 0;
	public:
		MultitextureSprite(const std::string& path, SDL_Renderer* renderer) noexcept;
		void render(SDL_Renderer* renderer) const noexcept;
		void renMove(int dx, int dy) noexcept;
	};
}