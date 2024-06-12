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
#include "Sprite.h"

namespace nv {
	class Renderer {
	protected: //EditorRenderer inherits Renderer
		SDL_Renderer* m_renderer;
		
		template<typename T>
		using Layers = FlatOrderedMap<int, plf::hive<T*>>;

		Layers<Sprite> m_multiSprites;
		Layers<Sprite> m_sprites;
		Layers<Rect> m_rects;
		Layers<Rect> m_borderRects;
		Layers<TextureData> m_textures;

		template<typename Sprites, typename Obj>
		void removeObjImpl(Sprites& objs, const Obj* const targetObj) {
			auto objPos = ranges::find_if(objs,
				[&](const Obj* const obj) { return obj == targetObj; }
			);
			assert(objPos != objs.end());
			objs.erase(objPos);
		}

		void renderCopyObjs();
	public:
		static constexpr int MIN_BOTTOM_LAYER = -1000;
		static constexpr int MAX_TOP_LAYER = 1000;

		bool showingCursor = true;

		Renderer(SDL_Renderer* renderer); 
		Renderer(const Renderer&) = delete;
		Renderer(Renderer&&)      = delete;

		inline SDL_Renderer* get() {
			return m_renderer;
		}

		void move(int dx, int dy) noexcept;

		void clear() noexcept;

		void add(Sprite* obj, int layer);
		void erase(const Sprite* const sprite, int layer);

		void add(Rect* rect, int layer);
		void removeRect(Rect* rect, int layer);

		void addBorderRect(Rect* rect, int layer);
		void removeBorderRect(const Rect * const rect, int layer);

		void add(TextureData* texData, int layer);
		void erase(const TextureData* const texData, int layer);

		void render() noexcept; 
		void renderWithImGui(ImGuiIO& io);
	};
}

#endif