#ifndef SCENE_H
#define SCENE_H

#include <nlohmann/json.hpp>

#include "Event.h"
#include "Instance.h"

#include "Renderer.h"

namespace nv {
	class Scene {
	private:
		SDL_Renderer* m_renderer;
		TextureMap m_texMap;
	public:
		bool running = false;

		Layers<Sprite> sprites;
		Layers<TextureObject> textures;

		EventHandler eventHandler;
		
		Scene(std::string_view path, SDL_Renderer* renderer);

		void operator()();
	};
}

#endif