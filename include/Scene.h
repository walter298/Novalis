#ifndef SCENE_H
#define SCENE_H

#include <algorithm>
#include <deque>
#include <fstream>
#include <memory>
#include <ranges>
#include <string>
#include <thread>
#include <vector>

#include <nlohmann/json.hpp>

#include "Event.h"
#include "Instance.h"

#include "Renderer.h"

namespace nv {
	class Scene {
	private:
		SDL_Renderer* m_renderer;
		void loadSpriteClones(const json& j, SDL_Renderer* renderer);
	public:
		template<typename T>
		using IDMap = std::vector<std::pair<std::string, std::vector<T>>>;

		using SpriteCloneData = std::pair<int, std::vector<std::vector<TextureData>>>;
		//using SpriteData  = IDMap<SpriteCloneData>;
		using TextureData = IDMap<TextureData>;

		bool running = false;

		Layers<Sprite> sprites;
		Layers<TextureObject> textures;

		EventHandler eventHandler;
		
		Scene(std::string_view path, SDL_Renderer* renderer);

		void operator()();
	};
}

#endif