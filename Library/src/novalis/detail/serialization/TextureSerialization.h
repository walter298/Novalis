#pragma once

#include <nlohmann/json.hpp>

#include "AggregateSerialization.h"
#include "KeyConstants.h"
#include "../../Instance.h"

namespace nlohmann {
	using namespace nv::detail::json_constants;

	template<>
	struct adl_serializer<nv::Texture> {
		static nv::Texture from_json(const json& j) {
			auto& instance = *nv::getGlobalInstance();
			auto& registry = instance.registry;

			//auto& objectJson = j[OBJECT_KEY];
			auto imagePath = j[IMAGE_PATH_KEY].get<std::string>();
			return {
				registry.loadTexture(instance.renderer, imagePath),
				j[RENDER_DATA_KEY].get<nv::TextureRenderData>()
			};
		}

		static void to_json(json& j, const nv::Texture& tex) {
			j[RENDER_DATA_KEY] = tex.texData;
		}
	};
}