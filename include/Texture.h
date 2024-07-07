#pragma once

#include <fstream>
#include <string_view>
#include <unordered_map>
#include <variant>

#include <SDL2/SDL_image.h>

#include "Rect.h"

namespace nv {
	struct TextureRAII {
		SDL_Texture* raw = nullptr;

		TextureRAII() = default;
		explicit TextureRAII(SDL_Texture* texture) noexcept;

		TextureRAII(const TextureRAII&)            = delete;
		TextureRAII& operator=(const TextureRAII&) = delete;

		~TextureRAII() noexcept;
	};

	using TextureMap = std::unordered_map<std::string, TextureRAII>;
	using TexturePtr = std::shared_ptr<TextureRAII>;

	struct TextureData {
		Rect ren;
		Rect world;
		SDL_Point rotationPoint{ 0, 0 };
		double angle = 0.0;
		SDL_RendererFlip flip = SDL_FLIP_NONE;
	};

	class TextureObject {
	private:
		std::variant<TexturePtr, SDL_Texture*> m_texVariant;
	public:
		TextureObject(std::string_view texPath, TexturePtr texPtr, TextureData texData);
		TextureObject(std::string_view texPath, SDL_Texture* rawTex, TextureData texData);
		TextureObject(SDL_Renderer* renderer, const json& json, TextureMap& texMap);

		std::shared_ptr<const std::string> texPath = nullptr;
		SDL_Texture* tex = nullptr;
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

		void save(json& json) const;
	};
}
