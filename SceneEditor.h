#pragma once

#include "Scene.h"
#include "EditorUtil.h"

namespace nv {
	namespace editor {
		class SceneEditor {
		private:
			SDL_Renderer* m_renderer;
			
			struct LoadedSprite : public Sprite, SharedIDObject {
				using TexturePaths = std::vector<std::string>;
				std::shared_ptr<TexturePaths> texPaths;
			};

			Layers<LoadedSprite> m_spriteLayers;
			Layers<LoadedTextureObject> m_localTextureLayers;
			Layers<Rect> m_rects;
			
			ObjectEditor<LoadedTextureObject, LoadedSprite> m_objEditor;

			int m_currLayer = 0;
			
			bool m_showingRightClickOptions = false;
			ImVec2 m_rightClickWinPos{ 0.0f, 0.0f };

			Layers<TextureData> getUniqueObjectData(const LoadedSprite& sprite);
			TextureData getUniqueObjectData(const LoadedTextureObject& texObj);

			template<typename ObjectRange, typename PMD>
			void saveObjectsWithSameID(json& objLayerNode, const ObjectRange& objsWithSameID, int layer, PMD sharedObjData,
				std::string_view sharedObjDataName, std::string_view uniqueObjDataArrayName) 
			{
				const auto& firstSprite = *ranges::begin(objsWithSameID);
				objLayerNode[sharedObjDataName] = *(firstSprite.*sharedObjData);
				objLayerNode["layer"] = layer;

				auto& objDataArr = objLayerNode[uniqueObjDataArrayName];
				objDataArr = json::array();
				for (const auto& obj : objsWithSameID) {
					objDataArr.push_back(getUniqueObjectData(obj));
				}
			}

			template<RenderObject Object, typename PM>
			void saveObjLayer(json& objLayerNode, std::vector<Object>& objLayer, int layer, PM sharedCloneData, 
				std::string_view sharedCloneDataName, std::string_view uniqueObjDataArrayName) 
			{
				ranges::sort(objLayer, [](const auto& obj1, const auto& obj2) {
					return obj1.getID() < obj2.getID();
				});
				auto saveObjectsWithSameIDCap = [&](const auto& objsWithSameID) {
					saveObjectsWithSameID(objLayerNode, objsWithSameID, layer, sharedCloneData, sharedCloneDataName, uniqueObjDataArrayName);
				};
				forEachEqualRange(objLayer, saveObjectsWithSameIDCap, [](const auto& sp1, const auto& sp2) {
					return sp1.getID() == sp2.getID();
				});
			}

			template<RenderObject Object, typename PM>
			void saveObjects(json& rootNode, std::string_view objsNodeName, Layers<Object>& objLayers, PM sharedCloneData, 
				std::string_view sharedCloneDataName, std::string_view uniqueObjDataArrayName)
			{
				auto& objLayersNode = rootNode[objsNodeName];
				objLayersNode = json::array();
				for (auto& [layer, objLayer] : objLayers) {
					saveObjLayer(objLayersNode.emplace_back(), objLayer, layer, sharedCloneData, sharedCloneDataName, uniqueObjDataArrayName);
				}
			}

			void loadSprite();
			void createTexture() noexcept;
			void showRightClickOptions() noexcept;
			void save() noexcept;
			void showSceneOptions() noexcept;
		public:
			SceneEditor(SDL_Renderer* renderer);
			EditorDest imguiRender();
			void sdlRender() const noexcept;
		};
	}
}

