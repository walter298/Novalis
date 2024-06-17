#pragma once

#include "Scene.h"
#include "EditorUtil.h"

namespace nv {
	namespace editor {
		class SceneEditor {
		private:
			Layers<Sprite> m_spriteLayers;
			Layers<TextureObject> m_textureLayers;
			Layers<TextureObjectAndPath> m_localTextureLayers;

			Layers<Rect> m_rects;
			
			ObjectEditor<TextureObject, TextureObjectAndPath, Sprite> m_objEditor;

			int m_currLayer = 0;

			bool m_showingRightClickOptions = false;
			ImVec2 m_rightClickWinPos{ 0.0f, 0.0f };

			template<RenderObject Obj>
			void insertObjFromFile(Renderer& renderer, plf::hive<Obj>& objs) {
				auto path = openFilePath();
				if (path) {
					try {
						auto spriteName = fileName(*path);
						objs.emplace(*path, renderer.get());
						renderer.add(&getBack(objs), m_currLayer);
					} catch (std::exception e) {
						std::println("{}", e.what());
					}
				}
			}
			void createTexture(Renderer& renderer) noexcept;
			void showRightClickOptions(Renderer& renderer) noexcept;
			void save() noexcept;
			void showSceneOptions() noexcept;
			void moveCamera(Renderer& renderer) noexcept;
		public:
			SceneEditor(Renderer& renderer);
			EditorDest operator()(Renderer& renderer);
		};
	}
}

