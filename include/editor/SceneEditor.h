#pragma once

#include "EditorUtil.h"
#include "ObjectSelection.h"
#include "PolygonEditor.h"
#include "../Sprite.h"

namespace nv {
	namespace editor {
		class SceneEditor {
		private:
			SDL_Renderer* m_renderer;
			TextureMap m_texMap;
			FontMap m_fontMap;
			
			ObjectLayers<EditedObjectData<Sprite>, EditedObjectData<Texture>, 
				EditedObjectData<Text>, EditedObjectData<Rect>, EditedObjectData<PolygonEditor>> m_objectLayers;

			ObjectSelection<Sprite, Texture, Text, Rect, PolygonEditor> m_objectSelection;

			struct SpecialPoint {
				static constexpr int RADIUS = 12;

				SDL_Point point;
				std::string name;

				bool containsCoord(SDL_Point mouse) const noexcept;
				void render(SDL_Renderer* renderer) const noexcept;
			};
			std::vector<SpecialPoint> m_specialPoints;

			enum class SelectedObjectType {
				Sprite,
				Texture,
				Text,
				Rect,
				SpecialPoint,
				None
			};


			//SelectedObjectType m_selectedObjType = SelectedObjectType::None;
			//TypeMap<SelectedObjectType, Sprite, Texture, Text, Rect, SpecialPoint> m_selectedObjMap;

			/*SelectedObjectData<Sprite> m_selectedSpriteData;
			SelectedObjectData<Texture> m_selectedTextureData;
			SelectedObjectData<Text> m_selectedTextData;
			SelectedObjectData<Rect> m_selectedRectData;*/
			SpecialPoint* m_selectedSpecialPoint = nullptr;

			bool m_draggingObject = false;
			bool m_selectedObj = false;

			//When inserting multiple textures, grid data
			int m_textureRowC    = 0;
			int m_textureColumnC = 0;

			bool m_showingFontOptions = false;
			int m_fontSize = 10;
			std::string m_fontPath;

			int m_currLayer = 0;
			
			bool m_showingRightClickOptions = false;
			ImVec2 m_rightClickWinPos{ 0.0f, 0.0f };

			int m_worldX = 0;
			int m_worldY = 0;

			void reduceOpacityOfOtherLayers();
			void selectSpecialPoint() noexcept;
			void editSelectedSpecialPoint();
			void createSpecialPoint();
			void createRect();
			void loadSprite();
			void showFontOptions();
			void createTextures() noexcept;
			void showRightClickOptions() noexcept;
			void save() const noexcept;
			void showSceneOptions() noexcept;
			void cameraMove(int dx, int dy);

			/*template<ranges::viewable_range Objects, typename Object>
			bool selectImpl(Objects& objs, Object& selectedObjData) {
				if (ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
					auto mouse = convertPair<SDL_Point>(ImGui::GetMousePos());
					auto selectedObjIt = std::ranges::find_if(objs, [&](const auto& editedObjData) -> bool {
						using HoveredResult = decltype(editedObjData.obj.containsCoord(mouse));
						if constexpr (IsClassTemplate<std::optional, HoveredResult>::value) {
							return editedObjData.obj.containsCoord(mouse).has_value();
						} else {
							return editedObjData.obj.containsCoord(mouse);;
						}
					});
					if (selectedObjIt != objs.end()) {
						selectedObjData.obj = &(*selectedObjIt);
						selectedObjData.objLayer = &objs;
						selectedObjData.it = selectedObjIt;

						using EditedObjectType = std::tuple_element_t<0, typename GetTemplateTypes<Object>::Types>;
						m_selectedObjType = m_selectedObjMap.get<EditedObjectType>();

						return true;
					} else {
						return false;
					}
				}
				return false;
			}*/

			void editLayers();
		public:
			SceneEditor(SDL_Renderer* renderer);
			EditorDest imguiRender();
			void sdlRender() const noexcept;
		};
	}
}