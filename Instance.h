#ifndef INSTANCE_H
#define INSTANCE_H

#include "Sound.h"
#include "Sprite.h"
#include "Texture.h"

namespace nv {
	class Instance {
	private:
		std::unordered_map<std::string, TextureDestructorWrapper> m_textures;
		void quit();
	public:
		SDL_Window* window     = nullptr;
		SDL_Renderer* renderer = nullptr;

		SDL_Texture* getTexture(std::string_view str);

		Instance(std::string_view windowTitle);
		Instance(Instance&&) = delete;
		~Instance() noexcept;
	};
}

#endif