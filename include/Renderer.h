#ifndef RENDERER_H
#define RENDERER_H

#include <chrono>
#include <map>
#include <string>
#include <filesystem>
#include <iostream>

#include <boost/container/flat_map.hpp>

#undef min
#undef max

#include <plf_hive.h>

#include <imgui.h>
#include <imgui_impl_sdlrenderer2.h>

#include "Camera.h"
#include "GlobalMacros.h"
#include "MultitextureSprite.h"
#include "Sprite.h"

namespace nv {
	class Renderer {
	protected: //EditorRenderer inherits Renderer
		SDL_Renderer* m_renderer;
		
		Background* m_background = nullptr;

		template<typename T>
		using Layers = FlatOrderedMap<int, plf::hive<T*>>;

		Layers<MultitextureSprite> m_multiSprites;
		Layers<Sprite> m_sprites;

		template<typename Sprites>
		void removeSpriteImpl(Sprites& sprites, const ID& ID, int layer) {
			auto& spriteHive = m_sprites.at(layer);
			auto spritePos = ranges::find_if(spriteHive,
				[&ID](const Sprite* const obj) { return obj->getID() == ID; }
			);
			assert(spritePos != spriteHive.end());
			spriteHive.erase(spritePos);
		}
	public:
		Renderer(SDL_Renderer* renderer); 
		Renderer(const Renderer&) = delete;
		Renderer(Renderer&&)      = delete;

		inline SDL_Renderer* get() {
			return m_renderer;
		}

		void move(int dx, int dy) noexcept;

		void clear() noexcept;

		void setBackground(Background* background) noexcept;

		void addSprite(Sprite* obj, int layer);
		void removeSprite(const Sprite* const sprite, int layer);

		void addSprite(MultitextureSprite* sprite, int layer);
		void removeSprite(const MultitextureSprite* const sprite, int layer);

		void render() noexcept; 
	};
}

#endif