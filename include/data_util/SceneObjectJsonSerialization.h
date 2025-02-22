#pragma once

#include <SDL3/SDL_render.h>
#include <nlohmann/json.hpp>

#include "../Texture.h"
#include "../Sprite.h"
//#include "../Text.h"
#include "../Instance.h"

namespace nlohmann {
	/*template<>
	struct nlohmann::adl_serializer<nv::Text> {
		static constexpr const char* TEXT_VALUE_KEY = "Text_Value";
		static constexpr const char* FONT_NAME_KEY = "Font_Name";
		static constexpr const char* FONT_SIZE_KEY = "Font_Size";

		static nv::Text from_json(const json& j) {
			auto& globalInstance = *nv::getGlobalInstance();
			auto& fontMap = globalInstance.fontMap;
			auto fontSize = j[FONT_SIZE_KEY].get<int>();
			auto fontName = j[FONT_NAME_KEY].get<std::string>();
			auto fontIt = fontMap.find(fontName);
			auto textValue = j[TEXT_VALUE_KEY].get<std::string>();

			if (fontIt == fontMap.end()) {
				auto font = nv::loadFont(fontName, fontSize);
				auto fontPtr = font.get();
				fontMap.emplace(std::move(fontName), std::move(font));
				return nv::Text{ globalInstance.renderer, textValue, fontSize, fontPtr };
			} else {
				auto fontPtr = fontIt->second.get();
				return nv::Text{ globalInstance.renderer, textValue, fontSize, fontPtr };
			}
		}

		static void to_json(json& j, const nv::Text& text) {
			text.save(j);
		}
	};*/

	/*template<>
	struct nlohmann::adl_serializer<nv::Sprite> {
		static nv::Sprite from_json(const json& j) {
			return nv::SceneObjectFactory::makeSprite(j);
		}

		static void to_json(json& j, const nv::Sprite& sprite) {
			sprite.save(j);
		}
	};*/

	template<>
	struct nlohmann::adl_serializer<nv::Texture> {
		static constexpr const char* IMAGE_PATH_KEY = "Image_Path";

		static nv::Texture from_json(const json& j) {
			auto& globalInstance = *nv::getGlobalInstance();
			auto& texMap = globalInstance.texMap;

			auto imagePath = j[IMAGE_PATH_KEY].get<std::string>();

			auto texIt = texMap.find(imagePath);
			if (texIt == texMap.end()) {
				nv::Texture tex{ globalInstance.renderer, imagePath.c_str() };
				texMap.emplace(std::move(imagePath), tex.tex);
				return tex;
			} else {
				auto tex = texMap.at(imagePath);
				return nv::Texture{ tex };
			}
		}

		static void to_json(json& j, const nv::Texture& tex) {
			//tex.save(j);
		}
	};
}