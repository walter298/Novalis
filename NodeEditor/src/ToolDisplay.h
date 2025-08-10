#pragma once

#include <boost/unordered/unordered_flat_map.hpp>
#include <novalis/Texture.h>

#include "imgui/imgui.h"
#include "WindowLayout.h"

namespace nv {
	namespace editor {
		enum class Tool {
			Move,
			ObjectSelect,
			ObjectGrab,
			AreaSelect,
			Polygon,
			Text,
			Delete
		};

		void loadToolTextures(SDL_Renderer* renderer);
		void destroyToolTextures(SDL_Renderer* renderer);
		void showToolDisplay(bool disabled);
		Tool getCurrentTool();
	}
}