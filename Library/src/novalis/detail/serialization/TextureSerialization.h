#pragma once

#include <nlohmann/json.hpp>

#include "AutoSerialization.h"
#include "KeyConstants.h"
#include "../../Instance.h"
#include "../TextureData.h"

namespace nlohmann {
	using namespace nv::detail::json_constants;

	template<>
	struct adl_serializer<nv::Texture> {
		static nv::Texture from_json(const json& j) {
			auto& instance = *nv::getGlobalInstance();
			auto& registry = instance.registry;

			auto imagePath = j[IMAGE_PATH_KEY].get<std::string>();
			return {
				registry.loadTexture(instance.getRenderer(), imagePath),
				j[RENDER_DATA_KEY].get<nv::detail::TextureRenderData>()
			};
		}
	};
}