#pragma once

#include "EditorUtil.h"

namespace nv {
	namespace editor {
		class SpriteEditor {
		private:
			FlatOrderedMap<int, plf::hive<TextureData>> m_textures;
			int m_currLayer = 0;
			int m_currLayoutLayer = 0;

			ObjectEditor m_texDataEditor;

			void makeUnselectedLayersInvisible();
			void showSpriteOptions(Renderer& renderer);
		public:
			SpriteEditor(Renderer& renderer) noexcept;
			EditorDest operator()(Renderer& renderer);
		};
	}
}


