#pragma once

#include "Scene.h"
#include "EditorUtil.h"

#include <print>

namespace nv {
	namespace editor {
		class SceneEditor {
		private:
			SDL_Renderer* m_renderer;

			Layers<Sprite> m_spriteLayers;
			Layers<TextureObject> m_textureLayers;
			Layers<TextureObjectAndPath> m_localTextureLayers;
			Layers<Rect> m_rects;
			
			ObjectEditor<TextureObject, TextureObjectAndPath, Sprite> m_objEditor;

			int m_currLayer = 0;

			bool m_showingRightClickOptions = false;
			ImVec2 m_rightClickWinPos{ 0.0f, 0.0f };

			/*template<RenderObject Obj>
			void insertObjFromFile(plf::hive<Obj>& objs) {
				auto path = openFilePath();
				if (path) {
					try {
						auto spriteName = fileName(*path);
						objs.emplace(*path, m_renderer);
					} catch (std::exception e) {
						std::println("{}", e.what());
					}
				}
			}*/
			void createTexture() noexcept;
			void showRightClickOptions() noexcept;
			void save() noexcept;
			void showSceneOptions() noexcept;
			void moveCamera(SDL_Renderer* renderer) noexcept;
		public:
			SceneEditor(SDL_Renderer* renderer);
			EditorDest imguiRender();
			void sdlRender() const noexcept;
		};
	}
}

