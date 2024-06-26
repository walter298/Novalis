#pragma once

#include <deque>

#include <hash_table7.hpp>

#include "EditorUtil.h"

namespace nv {
	namespace editor {
		class SpriteEditor {
		private:
			SDL_Renderer* m_renderer;

			Layers<TextureObjectAndPath> m_texLayers;
			size_t m_currLayer = 0;
			int m_currLayoutLayer = 0;

			ObjectEditor<TextureObjectAndPath> m_texDataEditor;
			
			void open(SDL_Renderer* renderer);
			void save();
			void insertTextures(SDL_Renderer* renderer);
			void setIdenticalLayout();
			void showSpriteOptions(SDL_Renderer* renderer);
		public:
			SpriteEditor(SDL_Renderer* renderer) noexcept;
			EditorDest imguiRender();
			void sdlRender() noexcept;
		};
	}
}


