#pragma once

#include <string_view>

#include "Rect.h"
#include "Namespace.h"

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

	struct TexturePos {
		static constexpr std::string_view renJkey = "ren";
		static constexpr std::string_view worldJkey = "world";
		static constexpr std::string_view angleJkey = "angle";
		static constexpr std::string_view rotationPointJkey = "rotation_point";
		static constexpr std::string_view flipJkey = "flip";

		Rect ren;
		Rect world;
		double angle = 0.0;
		SDL_Point rotationPoint{ 0, 0 };
		SDL_RendererFlip flip = SDL_FLIP_NONE;
	};

	/*void from_json(const json& j, TexturePos& pos);
	void to_json(json& j, const TexturePos& pos);*/
}