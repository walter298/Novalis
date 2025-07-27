#pragma once

#include <optional>
#include <string>
#include <thread>
#include <vector>

#include <novalis/detail/reflection/TypeMap.h>

#include "imgui/imgui.h"
#include "NodeSerialization.h"
#include "ObjectGroupManager.h"
#include "Layer.h"
#include "ObjectGroupCreator.h"
#include "PolygonOutline.h"
#include "PolygonBuilder.h"
#include "ToolDisplay.h"
#include "WindowLayout.h"
#include "NodeSerialization.h"
#include "FileID.h"

namespace nv {
	namespace editor {
		class NodeEditor {
		private:
			float m_zoom = 1.0f;
			float m_worldOffsetX = 0.0;
			float m_worldOffsetY = 0.0;
			SDL_FRect m_viewport;
			std::string m_name;
			std::string m_lastSavedFilePath;
			bool m_selectingObject = true;
			PolygonBuilder m_polygonBuilder;
			NameManager m_objectNameManager{ "Unnamed Object" };
			NameManager m_objectGroupNameManager{ "Unnamed Object Group" };
			NameManager m_layerNameManager{ "Unnamed Layer" };
			ObjectGroupManager m_objectGroupManager;
			ObjectGroupCreator m_objectGroupCreator;
			std::vector<Layer> m_layers;
			ObjectSearch m_objectSearch;
			size_t m_currLayerIdx = 0;
			bool m_draggingObject = true;
			nv::detail::TypeMap<bool, BufferedNode, DynamicPolygon, Texture, Spritesheet> m_objectSelectionFilter{ true };
			bool m_creatingObjectGroup = false;
			uint8_t m_externalLayerOpacity = 90;
			FileID m_id;

			template<typename Object>
			struct SelectedObjectData {
				EditedObjectData<Object>* obj = nullptr;
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
			void showObjectGroupsOfSelectedObject(EditedObjectData<Object>& object) {
				bool wasCreatingObjectGroup = m_creatingObjectGroup;
				m_objectGroupManager.showObjectGroupsOfObject(object, m_creatingObjectGroup);
				if (!wasCreatingObjectGroup && m_creatingObjectGroup) {
					m_objectGroupCreator.setNewObjects(m_layers);
					ImGui::OpenPopup(OBJECT_GROUP_CREATION_WINDOW_NAME);
				}
			}

			void createCollisionOutlines(SDL_Renderer* renderer, ID<EditedObjectGroup> groupID, EditedObjectData<Texture>& editedTex);
			void showCollisionOutlineOption(SDL_Renderer* renderer, EditedObjectData<Texture>& editedTex);
			void selectObject(SDL_Renderer* renderer, Point mouse);
			void makeCurrLayerMoreVisible();
			void editNodeName();
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
			void editSelectedObject(SDL_Renderer* renderer, Point mouse, ToolDisplay::GrabberTool& grabber);
			void runCurrentTool(SDL_Renderer* renderer, Point mouse, ToolDisplay& toolDisplay);
			void showObjectGroupCreationWindow();
			void showNodeWindow(SDL_Renderer* renderer, Point mouse);
		public:
			NodeEditor(FileID id, const std::string& name = "")
				: m_id{ id }, m_name{ name }
			{
				m_viewport = getViewport(m_zoom);
			}
			
			static std::optional<NodeEditor> load(FileID id);

			void show(SDL_Renderer* renderer, ToolDisplay& toolDisplay);
			void addLayer(std::string layerName);
			NodeSerializationResult serialize() const;

			template<typename Object>
			EditedObjectData<Object>& transfer(EditedObjectData<Object>&& object) {
				object.obj.setScreenPos({ 0.0f, 0.0f });
				object.obj.setWorldPos({ 0.0f, 0.0f });
				auto& objects = std::get<EditedObjectHive<Object>>(m_layers[m_currLayerIdx].objects);
				auto insertedObjectIt = objects.insert(std::move(object));
				m_selectedObject = SelectedObjectData{ &(*insertedObjectIt), &objects, insertedObjectIt };
				return *insertedObjectIt;
			}

			template<ranges::viewable_range Objects>
			void transfer(Objects& objects) {
				for (auto& object : objects) {
					transfer(std::move(object));
				}
			}

			void createObjectGroup() {
				m_creatingObjectGroup = true;
				m_objectGroupCreator.setNewObjects(m_layers);
				ImGui::OpenPopup(OBJECT_GROUP_CREATION_WINDOW_NAME);
			}

			void deselectSelectedObject() noexcept {
				m_selectedObject = std::monostate{};
				m_draggingObject = false;
			}

			bool hasNoLayers() const noexcept {
				return m_layers.empty();
			}

			bool isBusy() const noexcept {
				return m_polygonBuilder.building() || m_creatingObjectGroup;
			}

			const char* getName() const noexcept {
				return m_name.c_str();
			}

			FileID getID() const noexcept {
				return m_id;
			}

			void saveAs();
			
			void save();
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