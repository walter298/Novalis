#pragma once

#include <optional>
#include <vector>

#include <novalis/detail/reflection/TypeMap.h>

#include "ObjectGroupManager.h"
#include "Layer.h"
#include "ObjectGroupCreator.h"
#include "PolygonBuilder.h"
#include "NodeLayout.h"
#include "NodeSerialization.h"
#include "FileID.h"

namespace nv {
	namespace editor {
		class ErrorPopup;

		class NodeEditor {
		private:
			using Layers = std::vector<Layer>;
			Layers m_layers;
			float m_zoom = 1.0f;
			float m_worldOffsetX = 0.0;
			float m_worldOffsetY = 0.0;
			SDL_FRect m_viewport;
			bool m_selectingObject = true;
			PolygonBuilder m_polygonBuilder;
			NameManager m_objectNameManager{ "Unnamed Object" };
			NameManager m_objectGroupNameManager{ "Unnamed Object Group" };
			NameManager m_layerNameManager{ "Unnamed Layer" };
			ObjectGroupManager m_objectGroupManager;
			ObjectGroupCreator m_objectGroupCreator;
			ObjectSearch m_objectSearch;
			size_t m_currLayerIdx = 0;
			bool m_draggingObject = true;
			nv::detail::TypeMap<bool, BufferedNode, DynamicPolygon, Texture, Spritesheet> m_objectSelectionFilter{ true };
			bool m_creatingObjectGroup = false;
			uint8_t m_externalLayerOpacity = 90;
			FileID m_id;
			
			using ChildNodeMap = boost::unordered_flat_map<FileID, ObjectMetadata<BufferedNode>*>;
			ChildNodeMap m_children;
			
			template<typename Object>
			struct SelectedObjectData {
				ObjectMetadata<Object>* obj = nullptr;
				EditedObjectHive<Object>* objLayer = nullptr;
				EditedObjectHive<Object>::iterator it;
			};

			using SelectedObjectVariant = std::variant<
				std::monostate,
				SelectedObjectData<DynamicPolygon>,
				SelectedObjectData<Texture>,
				SelectedObjectData<BufferedNode>,
				SelectedObjectData<Spritesheet>
			>;
			SelectedObjectVariant m_selectedObject = std::monostate{};

			template<typename Object>
			void showObjectGroupsOfSelectedObject(ObjectMetadata<Object>& object) {
				bool wasCreatingObjectGroup = m_creatingObjectGroup;
				m_objectGroupManager.showObjectGroupsOfObject(object, m_creatingObjectGroup);
				if (!wasCreatingObjectGroup && m_creatingObjectGroup) {
					m_objectGroupCreator.setNewObjects(m_layers);
					ImGui::OpenPopup(OBJECT_GROUP_CREATION_WINDOW_NAME);
				}
			}

			void createCollisionOutlines(SDL_Renderer* renderer, ID<EditedObjectGroup> groupID, ObjectMetadata<Texture>& editedTex);
			void showCollisionOutlineOption(SDL_Renderer* renderer, ObjectMetadata<Texture>& editedTex);
			void selectObject(SDL_Renderer* renderer, Point mouse);
			void makeCurrLayerMoreVisible();
			void editLayerName();
			void selectLayer();
			void showObjectFilterOptions();
			void showExternalLayerOpacityOption();
			void showNodeOptions(bool disabled);
			void zoom(SDL_Renderer* renderer, Point mouse);
			void scroll();
			void dragSelectedObject(Point mouse);
			void editPolygon(SDL_Renderer* renderer, Point mouse);
			void render(SDL_Renderer* renderer) const noexcept;
			void editSelectedObject(SDL_Renderer* renderer, Point mouse);
			void runCurrentTool(SDL_Renderer* renderer, Point mouse);
			void showObjectGroupCreationWindow();
			void showNodeWindow(SDL_Renderer* renderer, Point mouse);
		public:
			NodeEditor(FileID id)
				: m_id{ id }
			{
				m_viewport = getViewport(m_zoom);
			}
			
			static std::optional<NodeEditor> load(const nlohmann::json& nodeJson, FileID id,
				ErrorPopup& errorPopup) noexcept;

			void updateNode(FileID fileID, BufferedNode node);
			void show(SDL_Renderer* renderer);
			void addLayer(std::string layerName);
			NodeSerializationResult serialize() const;

		private:
			template<typename Object>
			ObjectMetadata<Object>& transferImpl(ObjectMetadata<Object>&& object) {
				object.obj.setScreenPos({ 0.0f, 0.0f });
				object.obj.setWorldPos({ 0.0f, 0.0f });
				auto& objects = std::get<EditedObjectHive<Object>>(m_layers[m_currLayerIdx].objects);
				auto insertedObjectIt = objects.insert(std::move(object));
				m_selectedObject = SelectedObjectData{ &(*insertedObjectIt), &objects, insertedObjectIt };
				return *insertedObjectIt;
			}
		public:
			template<typename Object>
			ObjectMetadata<Object>& transfer(ObjectMetadata<Object>&& object) 
				requires(!std::same_as<Object, BufferedNode>) 
			{
				return transferImpl(std::move(object));
			}
			ObjectMetadata<BufferedNode>& transfer(FileID childID, ObjectMetadata<BufferedNode>&& node);

			template<ranges::viewable_range Objects>
			void transfer(Objects& objects) {
				for (auto& object : objects) {
					transfer(std::move(object));
				}
			}
			void createObjectGroup();
			void deselectSelectedObject() noexcept;
			bool hasNoLayers() const noexcept;
			bool isBusy() const noexcept;
			FileID getID() const noexcept;
		};
	}
}

namespace boost {
	template<>
	struct hash<nv::editor::NodeEditor> {
		size_t operator()(const nv::editor::NodeEditor& nodeEditor) const noexcept {
			return std::hash<nv::editor::FileID>{}(nodeEditor.getID());
		}
	};
}