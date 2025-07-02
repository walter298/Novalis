//#pragma once
//
//#include <nlohmann/json.hpp>
//
//#include "../TextureData.h"
//
//namespace nlohmann {
//	template<>
//	struct adl_serializer<nv::detail::TextureRenderData> {
//		static constexpr const char* REN_KEY = "Ren";
//		static constexpr const char* WORLD_KEY = "World";
//		static constexpr const char* FLIP_KEY = "Flip";
//		static constexpr const char* SCREEN_ROTATION_KEY = "Screen_Rotation_Data";
//		static constexpr const char* WORLD_ROTATION_KEY = "World_Rotation_Data";
//
//		static void from_json(const json& j, nv::detail::TextureRenderData& renderData) {
//			renderData.ren = j[REN_KEY].get<nv::Rect>();
//
//			renderData.flip = j[FLIP_KEY].get<SDL_FlipMode>();
//		}
//	};
//}