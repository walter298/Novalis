#pragma once

#include <nlohmann/json.hpp>

#include "../../Instance.h"
#include "../../Spritesheet.h"
#include "AutoSerialization.h"
#include "KeyConstants.h"

namespace nlohmann {
	template<>
	struct adl_serializer<nv::Spritesheet> {
		static constexpr const char* ROWS_KEY = "Rows";
		static constexpr const char* COLS_KEY = "Cols";

		static nv::Spritesheet from_json(const json& j) {
			using namespace nv::detail::json_constants;

			//get the render data
			auto renderData = j[RENDER_DATA_KEY].get<nv::detail::TextureRenderData>();

			//load the texture
			auto texPath = j[IMAGE_PATH_KEY].get<std::string>();
			auto instance = nv::getGlobalInstance();
			auto& registry = instance->registry;
			auto renderer = instance->getRenderer();
			auto tex = registry.loadTexture(renderer, texPath);

			//get row/column data
			auto rows = j[ROWS_KEY].get<int>();
			auto cols = j[COLS_KEY].get<int>();

			return nv::Spritesheet{ tex, renderData, rows, cols };
		}
	};
}