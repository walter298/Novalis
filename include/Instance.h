#ifndef INSTANCE_H
#define INSTANCE_H

#include "Arena.h"
#include "Renderer.h"
#include "Texture.h"

#include "Sound.h"

namespace nv {
	class Instance {
	private:
		//Arena m_arena;
		std::unordered_map<std::string, std::vector<Texture>> m_textures;

		void quit();
	public:
		SDL_Window* window;
		SDL_Renderer* renderer;

		Instance(std::string windowTitle);
		~Instance();

		//key is not string_view because you can't index string_view into map where key is std::string
		const std::vector<Texture>& getTextures(const std::string& key) const; 
		
		SDL_Window* getRawWindow() noexcept;
		SDL_Renderer* getRawRenderer() noexcept;
	};
}

#endif