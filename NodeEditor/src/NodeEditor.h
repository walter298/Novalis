#pragma once

#include <atomic>
#include <optional>
#include <string>
#include <thread>
#include <vector>
#include <SDL3/SDL_rect.h>

#include <novalis/detail/reflection/TypeMap.h>
#include <novalis/detail/serialization/BufferedNodeSerialization.h>

#include "imgui/imgui.h"
#include "NodeSerialization.h"
#include "ObjectGroupManager.h"
#include "Layer.h"
#include "ObjectGroupCreator.h"
#include "PolygonOutline.h"
#include "PolygonBuilder.h"
#include "ProgressBar.h"
#include "ToolDisplay.h"
#include "WindowLayout.h"

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
			ObjectGroupManager m_objectGroupManager;
			ObjectGroupCreator m_objectGroupCreator;
			std::vector<Layer> m_layers;
			ObjectSearch m_objectSearch;
			size_t m_currLayerIdx = 0;
			int m_worldX = 0;
			int m_worldY = 0;
			bool m_draggingObject = true;
			nv::detail::TypeMap<bool, BufferedNode, DynamicPolygon, Texture> m_objectSelectionFilter{ true };
			bool m_creatingObjectGroup = false;
			
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
				SelectedObjectData<BufferedNode>
			>;
			SelectedObjectVariant m_selectedObject = std::monostate{};

			template<typename Object>
			void moveObjectByMouseDragDelta(SelectedObjectData<Object>& editedObj, Point mousePos) {
				if (editedObj.obj->obj.containsCoord(mousePos)) {
					auto mouseChange = toSDLFPoint(ImGui::GetMouseDragDelta());
					editedObj.obj->obj.move(mouseChange);
					ImGui::ResetMouseDragDelta();
				}
			}

			template<typename Object>
			void showObjectRotationOption(SelectedObjectData<Object>& editedObj) {
				/*ImGui::SetNextItemWidth(getInputWidth());
				ImGui::Text("Rotation");
				auto floatAngle = static_cast<float>(editedObj.obj->angle);
				ImGui::SetNextItemWidth(getInputWidth());
				if (ImGui::SliderFloat("Angle", &floatAngle, 0.0f, 360.0f)) {
					editedObj.obj->obj.rotate(static_cast<double>(floatAngle), editedObj.obj->rotationPoint);
				}
				ImGui::SetNextItemWidth(getInputWidth());
				if (ImGui::InputInt("Rotation x", &editedObj.obj->rotationPoint.x)) {
					editedObj.obj->obj.rotate(static_cast<double>(floatAngle), editedObj.obj->rotationPoint);
				}
				ImGui::SetNextItemWidth(getInputWidth());
				if (ImGui::InputInt("Rotation y", &editedObj.obj->rotationPoint.y)) {
					editedObj.obj->obj.rotate(static_cast<double>(floatAngle), editedObj.obj->rotationPoint);
				}
				editedObj.obj->angle = static_cast<double>(floatAngle);*/
			}

			template<typename Object>
			void showObjectDuplicationOption(SelectedObjectData<Object>& editedObj) {
				ImGui::SetNextItemWidth(getInputWidth());
				if (ImGui::Button("Duplicate")) {
					/*assert(editedObj.objLayer);
					auto copiedObjectIt = editedObj.objLayer->insert(*editedObj.obj);
					editedObj.obj = &(*copiedObjectIt);
					editedObj.it = copiedObjectIt;*/
				}
			}

			template<typename Object>
			void showObjectDeletionOption(SelectedObjectData<Object>& editedObj) {
				ImGui::SetNextItemWidth(getInputWidth());
				if (ImGui::Button("Delete")) {
					m_objectGroupManager.removeFromAllObjectGroups(*editedObj.obj);
					editedObj.objLayer->erase(editedObj.it);
					editedObj.obj = nullptr;
					deselectSelectedObject();
				}
			}

			template<typename Object>
			void showOpacityOption(EditedObjectData<Object>& editedObj) {
				ImGui::SetNextItemWidth(getInputWidth());
				auto opacityInt = static_cast<int>(editedObj.opacity);
				if (ImGui::SliderInt("Opacity", &opacityInt, 0, 255)) {
					m_objectGroupManager.setOpacity(editedObj, static_cast<uint8_t>(opacityInt));
				}
			}

			template<concepts::SizeableObject Object>
			void showSizeOption(SelectedObjectData<Object>& editedObj) {
				ImGui::SetNextItemWidth(getInputWidth());
				if (ImGui::InputFloat("Width", &editedObj.obj->width)) {
					ImGui::SetNextItemWidth(getInputWidth());
					editedObj.obj->obj.setSize(editedObj.obj->width, editedObj.obj->height);
				}
				ImGui::SetNextItemWidth(getInputWidth());
				if (ImGui::InputFloat("Height", &editedObj.obj->height)) {
					ImGui::SetNextItemWidth(getInputWidth());
					editedObj.obj->obj.setSize(editedObj.obj->width, editedObj.obj->height);
				}
				ImGui::SetNextItemWidth(getInputWidth());
				if (ImGui::Button("Set to screen size")) {
					auto nodeWinSize = toSDLFPoint(ImGui::FindWindowByName(NODE_WINDOW_NAME)->Size);
					editedObj.obj->obj.setSize(nodeWinSize);
				}
			}

			template<concepts::ScaleableObject Object>
			void showScaleOption(EditedObjectData<Object>& editedObj) {
				ImGui::SetNextItemWidth(getInputWidth());
				if (ImGui::SliderFloat("Scale", &editedObj.scale, 1.0f, 5.0f)) {
					m_objectGroupManager.scale(editedObj, editedObj.scale);
				}
			}

			template<concepts::MoveableObject Object>
			void showXYCoordinateOption(SelectedObjectData<Object>& editedObj) {
				ImGui::SetNextItemWidth(getInputWidth());
				bool changedX = ImGui::InputFloat("x", &editedObj.obj->x);
				ImGui::SetNextItemWidth(getInputWidth());
				bool changedY = ImGui::InputFloat("y", &editedObj.obj->y);

				if (changedX || changedY) {
					auto [wx, wy] = editedObj.obj->obj.getWorldPos();
					Point change{ editedObj.obj->x - wx, editedObj.obj->y - wy };
					editedObj.obj->obj.move(change);
				}
			}

			void createCollisionOutlines(SDL_Renderer* renderer, ID<EditedObjectGroup> groupID, 
				EditedObjectGroup& collisionGroup, EditedObjectData<Texture>& editedTex) 
			{
				editedTex.groupIDs.insert(groupID);
				collisionGroup.addObject(&editedTex);

				auto collisionOutlines = getPolygonOutlines(renderer, editedTex.obj, m_worldOffsetX, m_worldOffsetY);
				for (auto& outline : collisionOutlines) {
					auto& object = transfer(EditedObjectData<DynamicPolygon>{ std::move(outline) });
					object.groupIDs.insert(groupID);
					collisionGroup.addObject(&object);
				}
			}

			void showCollisionOutlineOption(SDL_Renderer* renderer, EditedObjectData<Texture>& editedTex) {
				ImGui::SetNextItemWidth(getInputWidth());
				if (ImGui::Button("Create collision outline")) {
					auto [spriteGroupID, spriteGroup] = m_objectGroupManager.addGroup();
					createCollisionOutlines(renderer, spriteGroupID, spriteGroup, editedTex);
				}
			}

			void showObjectGroupCreationWindow() {
				if (!m_objectGroupCreator.show(m_objectGroupManager, m_objectGroupNameManager)) {
					m_creatingObjectGroup = false;
				}
			}

			template<nv::concepts::RenderableObject Object>
			void edit(SDL_Renderer* renderer, Point mousePos, SelectedObjectData<Object>& editedObj) {
				ImGui::BeginDisabled(isBusy());

				//show opacity
				showOpacityOption(*editedObj.obj);

				//setting size
				if constexpr (concepts::SizeableObject<Object>) {
					showSizeOption(editedObj);
				}

				//scaling texture
				if constexpr (concepts::ScaleableObject<Object>) {
					showScaleOption(*editedObj.obj);
				}

				//rotation
				if constexpr (concepts::RotateableObject<Object>) {
					showObjectRotationOption(editedObj);
				}

				//name input
				editedObj.obj->inputName();
				
				//duplication 
				if constexpr (std::copy_constructible<Object>) {
					showObjectDuplicationOption(editedObj);
				}

				//x-y coordinates
				if constexpr (concepts::MoveableObject<Object>) {
					showXYCoordinateOption(editedObj);
				}

				//collision outline
				if constexpr (std::same_as<Object, Texture>) {
					showCollisionOutlineOption(renderer, *editedObj.obj);
				}

				//deletion
				showObjectDeletionOption(editedObj);

				ImGui::EndDisabled();
			}

			void selectObject(SDL_Renderer* renderer, Point mouse) {
				if (!ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
					return;
				}

				nv::detail::forEachDataMember([&, this]<typename Object>(EditedObjectHive<Object>& currObjectHive) {
					if (!m_objectSelectionFilter.get<Object>()) {
						return nv::detail::STAY_IN_LOOP;
					}

					for (auto it = currObjectHive.begin(); it != currObjectHive.end(); it++) {
						auto& editedObjData = *it;
						if (editedObjData.obj.containsScreenCoord(mouse)) {
							editedObjData.obj.setOpacity(255);
							m_selectedObject = SelectedObjectData{
								&(*it), &currObjectHive, it
							};
						} else {
							editedObjData.obj.setOpacity(100);
						}
					}
					return nv::detail::STAY_IN_LOOP;
				}, m_layers[m_currLayerIdx].objects);
			}

			void makeCurrLayerMoreVisible() {
				auto setOpacityImpl = [](auto& hiveTuple, uint8_t opacity) {
					nv::detail::forEachDataMember([&](auto& objHive) {
						for (auto& editedObj : objHive) {
							editedObj.obj.setOpacity(opacity);
						}
						return nv::detail::STAY_IN_LOOP;
					}, hiveTuple);
				};

				auto setOpacity = [&, this](auto layers, uint8_t opacity) {
					for (auto& [layerName, objects] : layers) {
						setOpacityImpl(objects, opacity);
					}
				};

				constexpr uint8_t REDUCED_OPACITY = 70;

				//reduce opacity of all layers before the current layer
				setOpacity(ranges::subrange(m_layers.begin(), m_layers.begin() + m_currLayerIdx), REDUCED_OPACITY);

				//reduce opacity of all layers after the current layer
				setOpacity(ranges::subrange(m_layers.begin() + m_currLayerIdx + 1, m_layers.end()), REDUCED_OPACITY);

				//make the current layer visible in case that its opacity was reduced
				setOpacityImpl(m_layers[m_currLayerIdx].objects, 255);
			}

			void editNodeName() {
				ImGui::SetNextItemWidth(getInputWidth());
				ImGui::InputText("Node Name", &m_name);
			}

			void editLayerName() {
				ImGui::SetNextItemWidth(getInputWidth());
				auto& currLayerName = m_layers[m_currLayerIdx].name;
				ImGui::InputText("Current Layer", &currLayerName);
			}

			void selectLayer() {
				ImGui::SetNextItemWidth(getInputWidth());

				const auto& topLayerName = m_layers[m_currLayerIdx].name;
				if (ImGui::BeginCombo("Layers", topLayerName.empty() ? "Current Layer" : topLayerName.c_str())) {
					for (auto [idx, layer] : std::views::enumerate(m_layers)) {
						auto& [layerName, objects] = layer;
						bool selected = std::cmp_equal(idx, m_currLayerIdx);

						if (layerName.empty()) {
							layerName = "Layer " + std::to_string(idx);
						}
						if (ImGui::Selectable(layerName.c_str(), selected)) {
							m_currLayerIdx = static_cast<size_t>(idx);
							makeCurrLayerMoreVisible();
						}
					}
					ImGui::EndCombo();
				}
			}

			void showObjectFilterOptions() {
				ImGui::SetNextItemWidth(getInputWidth());
				ImGui::Text("Object Filters");
				auto showSelectionOption = [this]<typename T>(const char* label, std::type_identity<T>) {
					ImGui::SetNextItemWidth(getInputWidth());
					auto& filtered = m_objectSelectionFilter.get<T>();
					auto temp = filtered;
					if (ImGui::Checkbox(label, &temp)) {
						filtered = !filtered;
						if (!filtered && std::holds_alternative<SelectedObjectData<T>>(m_selectedObject)) {
							m_selectedObject = std::monostate{};
						}
					}
				};
				showSelectionOption("Nodes", std::type_identity<BufferedNode>{});
				showSelectionOption("Polygons", std::type_identity<DynamicPolygon>{});
				showSelectionOption("Textures", std::type_identity<Texture>{});
			}

			void showNodeOptions(bool disabled) {
				ImGui::BeginDisabled(disabled);

				ImGui::SetNextWindowPos({ getNodeOptionsWindowPos(), });
				ImGui::SetNextWindowSize({ getSideWindowWidth(), getWindowHeight() });
				ImGui::Begin(NODE_OPTIONS_WINDOW_NAME);

				showObjectFilterOptions();
				editNodeName();
				editLayerName();
				selectLayer();
				m_objectGroupManager.showAllObjectGroups(m_objectGroupNameManager);

				ImGui::End();

				ImGui::EndDisabled();
			}

			void zoom(SDL_Renderer* renderer, Point mouse) {
				auto& io = ImGui::GetIO();
				if (ImGui::IsKeyDown(ImGuiKey_LeftCtrl) && io.MouseWheel != 0.0f) {
					auto dZoom = io.MouseWheel / 10.0f;
					m_zoom += dZoom;
					m_viewport = getViewport(m_zoom);
				}
			}

			void scroll() {
				auto change = toSDLFPoint(ImGui::GetMouseDragDelta());
				m_worldOffsetX += change.x;
				m_worldOffsetY += change.y;
				for (auto& [name, objects] : m_layers) {
					nv::detail::forEachDataMember([&](auto& objHive) {
						for (auto& obj : objHive) {
							obj.obj.screenMove(change);
						}
						return nv::detail::STAY_IN_LOOP;
					}, objects);
				}
				ImGui::ResetMouseDragDelta();
			}

			template<typename Callable, typename Variant>
			static void selectiveVisit(Callable callable, Variant& variant) {
				std::visit([&callable](auto& visited) {
					if constexpr (!std::same_as<std::monostate, std::remove_cvref_t<decltype(visited)>>) {
						std::invoke(callable, visited);
					}
				}, variant);
			}

			void dragSelectedObject(Point mouse) {
				Point change = toSDLFPoint(ImGui::GetMouseDragDelta());
				change /= m_zoom;
				selectiveVisit([&](auto& selectedObject) {
					if (selectedObject.obj->obj.containsScreenCoord(mouse) || ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
						m_objectGroupManager.move(*selectedObject.obj, change);
					}
				}, m_selectedObject);
				ImGui::ResetMouseDragDelta();
			}

			void editPolygon(SDL_Renderer* renderer, Point mouse) {
				auto polygon = m_polygonBuilder(renderer, mouse, m_worldOffsetX, m_worldOffsetY);
				if (polygon) {
					auto& polygons = std::get<EditedObjectHive<DynamicPolygon>>(m_layers[m_currLayerIdx].objects);
					auto insertedPolygonIt = polygons.insert(std::move(*polygon));
					m_selectedObject = SelectedObjectData{ &(*insertedPolygonIt), &polygons, insertedPolygonIt };
				}
			}

			void render(SDL_Renderer* renderer) const noexcept {
				for (auto& [layerName, objects] : m_layers) {
					nv::detail::forEachDataMember([&, this](const auto& hive) {
						for (const auto& editedObj : hive) {
							editedObj.obj.render(renderer);
						}
						return nv::detail::STAY_IN_LOOP;
					}, objects);
				}
			}

			void editSelectedObject(SDL_Renderer* renderer, Point mouse, ToolDisplay::GrabberTool& grabber) {
				auto renderAdjustedMouse = getViewportAdjustedMouse(renderer, mouse, m_zoom);
				selectObject(renderer, renderAdjustedMouse);
				dragSelectedObject(renderAdjustedMouse);
			}

			void runCurrentTool(SDL_Renderer* renderer, Point mouse, ToolDisplay& toolDisplay) {
				if (toolDisplay.getCurrentTool() == Tool::ObjectSelect && m_selectingObject) {
					makeCurrLayerMoreVisible();
				}
				switch (toolDisplay.getCurrentTool()) {
				case Tool::Move:
					scroll();
					break;
				case Tool::Polygon:
					editPolygon(renderer, getViewportAdjustedMouse(renderer, mouse, m_zoom));
					break;
				case Tool::ObjectSelect:
					editSelectedObject(renderer, mouse, toolDisplay.grabber);
					break;
				}
			}

			void configureNodeWindow() {
				auto toolWindow = ImGui::FindWindowByName(TOOL_WINDOW_NAME);
				auto toolWindowPos = toolWindow->Pos;
				auto toolWindowSize = toolWindow->Size;

				ImVec2 nodeWindowPos{ toolWindowPos.x, toolWindowPos.y + toolWindowSize.y + 0.5f };
				ImVec2 nodeWindowSize{ toolWindowSize.x, getWindowHeight() - toolWindowSize.y };
				ImGui::SetNextWindowPos(nodeWindowPos);
				ImGui::SetNextWindowSize(nodeWindowSize);
			}

			template<typename Object>
			void showObjectGroupsOfSelectedObject(EditedObjectData<Object>& object) {
				bool wasCreatingObjectGroup = m_creatingObjectGroup;
				m_objectGroupManager.showObjectGroupsOfObject(object, m_creatingObjectGroup);
				if (!wasCreatingObjectGroup && m_creatingObjectGroup) {
					m_objectGroupCreator.setNewObjects(m_layers);
					ImGui::OpenPopup(OBJECT_GROUP_CREATION_WINDOW_NAME);
				}
			}

			void showNodeWindow(SDL_Renderer* renderer, Point mouse) {
				configureNodeWindow();

				ImGui::Begin(OBJECT_WINDOW_NAME, nullptr, WINDOW_FLAGS);

				selectiveVisit([&, this](auto& selectedObject) {
					if (selectedObject.obj != nullptr) {
						edit(renderer, mouse, selectedObject);
						showObjectGroupsOfSelectedObject(*selectedObject.obj);
					}
				}, m_selectedObject);

				ImGui::End();
			}
		public:
			NodeEditor(const std::string& name = "")
				: m_name{ name }, m_viewport{ getViewport(m_zoom) }
			{
			}
		
			static std::optional<NodeEditor> load() {
				auto filePath = openFile({ { "node", "nv_node" } });
				if (!filePath) {
					return std::nullopt;
				}
				std::ifstream file{ *filePath };
				if (!file.is_open()) {
					std::println(stderr, "Error: could not open {}", *filePath);
					return std::nullopt;
				}
				auto nodeJson = nlohmann::json::parse(file);

				NodeEditor ret{ fileName(*filePath) };

				auto& layersJson = nodeJson[LAYERS_KEY];
				ret.m_layers.reserve(nodeJson.size());

				for (const auto& layerJson : nodeJson[LAYERS_KEY]) {
					auto layerName = layerJson[NAME_KEY].get<std::string>();
					auto& currLayer = ret.m_layers.emplace_back(layerName);

					nv::detail::forEachDataMember([&layerJson]<typename Object>(EditedObjectHive<Object>& objectGroup) {
						auto typeName = nv::detail::getTypeName<BufferedObject<Object>>();
						auto objectGroupJsonIt = layerJson.find(typeName);
						
						if (objectGroupJsonIt == layerJson.end()) {
							return nv::detail::STAY_IN_LOOP;
						}

						auto& objectGroupJson = *objectGroupJsonIt;
						for (const auto& objectJson : objectGroupJson) {
							objectGroup.insert(EditedObjectData<Object>::load(objectJson));
						}
						return nv::detail::STAY_IN_LOOP;
					}, currLayer.objects);
				}

				return ret;
				//return std::nullopt;
			}

			void show(SDL_Renderer* renderer, ToolDisplay& toolDisplay) {
				if (m_layers.empty()) {
					return;
				}

				auto viewportIntRect = toSDLRect(m_viewport);
				SDL_SetRenderViewport(renderer, &viewportIntRect);
				SDL_RenderClear(renderer);
				SDL_SetRenderScale(renderer, m_zoom, m_zoom);

				auto mouse = toSDLFPoint(ImGui::GetMousePos());

				showNodeWindow(renderer, mouse);
				zoom(renderer, mouse);
				render(renderer);

				if (windowContainsCoord(NODE_WINDOW_NAME, mouse)) {
					runCurrentTool(renderer, mouse, toolDisplay);
				}

				viewportIntRect = toSDLRect(getViewport(1.0f));
				SDL_SetRenderViewport(renderer, &viewportIntRect);
				SDL_SetRenderScale(renderer, 1.0f, 1.0f);

				if (m_creatingObjectGroup) {
					showObjectGroupCreationWindow();
				}
				showNodeOptions(m_creatingObjectGroup);

				SDL_SetRenderViewport(renderer, nullptr);
			}

			void addLayer(const std::string& layerName) {
				m_selectedObject = std::monostate{};

				if (m_layers.empty()) {
					m_layers.emplace_back(layerName);
				} else {
					m_layers.emplace(m_layers.begin() + m_currLayerIdx + 1, layerName);
				}
			}

			template<ranges::viewable_range Objects>
			void transfer(Objects& objects) {
				std::get<plf::hive<typename Objects::value_type>>(m_layers[m_currLayerIdx].objects).insert(
					std::make_move_iterator(objects.begin()),
					std::make_move_iterator(objects.end())
				);
			}

			template<typename Object>
			EditedObjectData<Object>& transfer(EditedObjectData<Object>&& object) {
				auto& objects = std::get<EditedObjectHive<Object>>(m_layers[m_currLayerIdx].objects);
				auto insertedObjectIt = objects.insert(std::move(object));
				m_selectedObject = SelectedObjectData{ &(*insertedObjectIt), &objects, insertedObjectIt };
				return *insertedObjectIt;
			}

			void createSpritesheet(SDL_Renderer* renderer, std::vector<EditedObjectData<Texture>> spriteSheet) {
				//make room for new layers containing each texture in the sprite sheet
				auto layersLeft = m_layers.size() - m_currLayerIdx;
				if (spriteSheet.size() > layersLeft) {
					m_layers.resize(m_layers.size() + (spriteSheet.size() - layersLeft) + 1);
				}

				int steps = static_cast<int>(spriteSheet.size());
				
				auto [spriteGroupID, spriteGroup] = m_objectGroupManager.addGroup();
				for (auto& texture : spriteSheet) {
					auto& currLayer = m_layers[m_currLayerIdx];
					auto& textures = std::get<EditedObjectHive<Texture>>(currLayer.objects);
					auto& insertedTex = *textures.insert(std::move(texture));
					
					createCollisionOutlines(renderer, spriteGroupID, spriteGroup, insertedTex);

					m_currLayerIdx++;
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

			bool isBusy() {
				return m_polygonBuilder.building() || m_creatingObjectGroup;
			}

			const char* getName() const noexcept {
				return m_name.c_str();
			}

			void saveAs() {
				try {
					saveNewFile({ { "node", "nv_node" } }, [this](const auto& path) {
						m_lastSavedFilePath = path;
						return createNodeJson(m_layers, m_name, m_objectGroupManager);
					});
				} catch (json::exception e) {
					std::println("{}", e.what());
				}
			}

			void save() {
				if (m_lastSavedFilePath.empty()) {
					saveAs();
				} else {
					try {
						saveToExistingFile(m_lastSavedFilePath, createNodeJson(m_layers, m_name, m_objectGroupManager));
					} catch (json::exception e) {
						std::println("{}", e.what());
					}
				}
			}
		};
	}
}