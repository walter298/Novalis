#pragma once

#include "Scene.h"
#include "EditorUtil.h"

namespace nv {
	namespace editor {
		class SceneEditor {
		private:
			SDL_Renderer* m_renderer;
			TextureMap m_texMap;

			Layers<EditedObjectData<Sprite>> m_spriteLayers;
			Layers<EditedObjectData<TextureObject>> m_texObjLayers;

			enum class SelectedObjectType {
				Sprite,
				Texture,
				None
			};
			SelectedObjectType m_selectedObjType = SelectedObjectType::None;
			SelectedObjectData<Sprite> m_selectedSpriteData;
			SelectedObjectData<TextureObject> m_selectedTextureData;
			
			int m_currLayer = 0;
			
			bool m_showingRightClickOptions = false;
			ImVec2 m_rightClickWinPos{ 0.0f, 0.0f };

			void loadSprite();
			void createTexture() noexcept;
			void showRightClickOptions() noexcept;
			void save() noexcept;
			void showSceneOptions() noexcept;
			void editLayers();
		public:
			SceneEditor(SDL_Renderer* renderer);
			EditorDest imguiRender();
			void sdlRender() const noexcept;
		};
	}
}

