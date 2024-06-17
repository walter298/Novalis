#ifndef SCENE_H
#define SCENE_H

#include <algorithm>
#include <fstream>
#include <memory>
#include <ranges>
#include <string>
#include <thread>
#include <vector>

#include <nlohmann/json.hpp>

#include "Event.h"
#include "Instance.h"
#include "Button.h"
#include "Sprite.h"

namespace nv {
	struct Scene {
		using SpriteData  = std::vector<std::pair<std::string, std::vector<Layers<TextureData, std::vector<TextureData>>>>>;
		using TextureData = std::pair<std::string, std::vector<TextureData>>;

		bool running = false;

		Layers<Sprite> sprites;
		Layers<Texture> textures;

		EventHandler eventHandler;
		Renderer renderer;

		Scene(std::string_view path, Instance& instance);

		void operator()();
	};
}

#endif