#pragma once

#include <hash_table7.hpp>

#include "EditorUtil.h"

namespace nv {
	namespace editor {
		class SpriteEditor {
		private:
			Layers<TextureDataAndPath> m_textures;
			int m_currLayer = 0;
			int m_currLayoutLayer = 0;

			ObjectEditor<TextureDataAndPath> m_texDataEditor;
			
			void open(Renderer& renderer);
			void save();
			void insertTextures(Renderer& renderer);
			void setIdenticalLayout();
			void showSpriteOptions(Renderer& renderer);
		public:
			SpriteEditor(Renderer& renderer) noexcept;
			EditorDest operator()(Renderer& renderer);
		};
	}
}


