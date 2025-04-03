#pragma once

#include <string>
#include <vector>
#include <SDL3/SDL_rect.h>

#include "../detail/serialization/BufferedNodeSerialization.h"
#include "../Node.h"
#include "../Polygon.h"
#include "EditedObjectData.h"
#include "SpecialPoint.h"
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

			class PolygonBuilder {
			private:
				std::vector<Point> m_screenPoints;
				bool m_placingNewPoint = false;

				SpecialPoint m_firstPoint;
				SpecialPoint m_lastPlacedPoint;
				bool m_building = false;

				DynamicPolygon createPolygon(float worldOffsetX, float worldOffsetY) {
					//make the first point of the polygon also be the last so that the shape forms a closed ring
					auto first = m_screenPoints.front();
					m_screenPoints.push_back(std::move(first));

					auto makeWorldPoints = m_screenPoints | std::views::transform([&](const Point& point) {
						return Point{ point.x + worldOffsetX, point.y + worldOffsetY };
					});
					std::vector<Point> worldPoints;
					worldPoints.append_range(makeWorldPoints);

					DynamicPolygon poly{ std::move(m_screenPoints), std::move(worldPoints) };
					m_screenPoints.clear();
					m_building = false;
					return poly;
				}
			public:
				bool building() const noexcept {
					return m_building;
				}

				std::optional<DynamicPolygon> operator()(SDL_Renderer* renderer, Point mouse, float worldOffsetX, float worldOffsetY) noexcept {
					if (!m_building) {
						m_screenPoints.clear();
					}

					ImGui::SetMouseCursor(ImGuiMouseCursor_None);
					m_lastPlacedPoint.point = mouse;
					m_lastPlacedPoint.render(renderer);

					if (!m_screenPoints.empty()) {
						m_firstPoint.point = m_screenPoints.front();
						m_firstPoint.render(renderer);
						detail::renderScreenPoints(renderer, 255, m_screenPoints);
					}

					if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
						m_building = true;
						if (m_firstPoint.containsCoord(mouse) && m_screenPoints.size() > 1) {
							return createPolygon(worldOffsetX, worldOffsetY);
						} else {
							m_screenPoints.emplace_back(mouse.x, mouse.y);
							m_lastPlacedPoint.point = mouse;
						}
					}
					return std::nullopt;
				}
			};
			PolygonBuilder m_polygonBuilder;

			template<typename Object>
			struct SelectedObjectData {
				EditedObjectData<Object>* obj = nullptr;
				EditedObjectHive<Object>* objLayer = nullptr;
				EditedObjectHive<Object>::iterator it;

				void resetToRandomElement(EditedObjectHive<Object>* newObjLayer) {
					obj = &(*newObjLayer->begin());
					objLayer = newObjLayer;
					it = newObjLayer->begin();
				}
				void reset() {
					obj = nullptr;
					objLayer = nullptr;
				}
				void select(EditedObjectHive<Object>& hive, EditedObjectHive<Object>::iterator it) {
					obj = &(*it);
					objLayer = &hive;
					this->it = it;
				}
			};

			using SelectedObjectVariant = std::variant<
				std::monostate,
				SelectedObjectData<DynamicPolygon>,
				SelectedObjectData<Texture>,
				SelectedObjectData<BufferedNode>
			>;
			SelectedObjectVariant m_selectedObject = std::monostate{};

			bool m_draggingObject = true;

			struct Layer {
				std::string name;
				using Objects = std::tuple<
					EditedObjectHive<DynamicPolygon>,
					EditedObjectHive<Texture>,
					EditedObjectHive<BufferedNode>
				>;
				Objects objects;
			};
			std::vector<Layer> m_layers;
			size_t m_currLayerIdx = 0;

			int m_worldX = 0;
			int m_worldY = 0;
			std::string m_worldXLabel;
			std::string m_worldYLabel;

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
					auto it = editedObj.objLayer->insert(*editedObj.obj);
					editedObj.obj = &(*it);
					editedObj.it = it;
				}
			}

			template<typename Object>
			void showObjectDeletionOption(SelectedObjectData<Object>& editedObj) {
				ImGui::SetNextItemWidth(getInputWidth());
				if (ImGui::Button("Delete")) {
					editedObj.objLayer->erase(editedObj.it);
					editedObj.obj = nullptr;
					deselectSelectedObject();
				}
			}

			template<typename Object>
			void showOpacityOption(SelectedObjectData<Object>& editedObj) {
				ImGui::SetNextItemWidth(getInputWidth());
				auto opacityInt = static_cast<int>(editedObj.obj->opacity);
				if (ImGui::SliderInt("Opacity", &opacityInt, 0, 255)) {
					editedObj.obj->opacity = static_cast<uint8_t>(opacityInt);
					editedObj.obj->obj.setOpacity(editedObj.obj->opacity);
				}
			}

			template<concepts::SizeableObject Object>
			void showSizeOption(SelectedObjectData<Object>& editedObj) {
				ImGui::SetNextItemWidth(getInputWidth());
				if (ImGui::InputFloat("width", &editedObj.obj->width)) {
					ImGui::SetNextItemWidth(getInputWidth());
					editedObj.obj->obj.setSize(editedObj.obj->width, editedObj.obj->height);
				}
				ImGui::SetNextItemWidth(getInputWidth());
				if (ImGui::InputFloat("height", &editedObj.obj->height)) {
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
			void showScaleOption(SelectedObjectData<Object>& editedObj) {
				ImGui::SetNextItemWidth(getInputWidth());
				if (ImGui::SliderFloat("Scale", &editedObj.obj->scale, 1.0f, 5.0f)) {
					editedObj.obj->obj.screenScale(editedObj.obj->scale);
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

			template<concepts::RenderableObject Object>
			void edit(SDL_Renderer* renderer, Point mousePos, SelectedObjectData<Object>& editedObj) {
				//if we are editing text
				/*if constexpr (std::same_as<Object, Text>) {
					ImGui::SetNextItemWidth(getInputWidth());
					std::string temp = editedObj.obj->obj.value().data();
					if (ImGui::InputText("Value", &temp)) {
						editedObj.obj->obj = temp;
					}
				}*/

				showOpacityOption(editedObj);

				ImGui::Text("Size");

				//setting size
				if constexpr (concepts::SizeableObject<Object>) {
					showSizeOption(editedObj);
				}

				//scaling texture
				if constexpr (concepts::ScaleableObject<Object>) {
					showScaleOption(editedObj);
				}

				//rotation
				if constexpr (concepts::RotateableObject<Object>) {
					showObjectRotationOption(editedObj);
				}

				ImGui::SetNextItemWidth(getInputWidth());
				if (ImGui::InputText("Name", &editedObj.obj->name)) {
					return;
				}

				//duplication 
				if constexpr (std::copyable<Object>) {
					showObjectDuplicationOption(editedObj);
				}

				//x-y coordinates
				if constexpr (concepts::MoveableObject<Object>) {
					showXYCoordinateOption(editedObj);
				}

				//deletion
				showObjectDeletionOption(editedObj);
			}

			void selectObject(SDL_Renderer* renderer, Point mouse, ToolDisplay::GrabberTool& grabber) {
				grabber.render(renderer, mouse);

				if (!ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
					return;
				}
				auto& objectHives = m_layers[m_currLayerIdx].objects;
				detail::forEachDataMember([&, this](auto& currObjectHive) {
					auto selectedObjIt = std::ranges::find_if(currObjectHive, [&](const auto& editedObjData) -> bool {
						return editedObjData.obj.containsScreenCoord(mouse);
						});
					if (selectedObjIt == currObjectHive.end()) {
						return detail::STAY_IN_LOOP;
					}
					m_selectedObject = SelectedObjectData{
						&(*selectedObjIt), &currObjectHive, selectedObjIt
					};
					return detail::BREAK_FROM_LOOP;
					}, objectHives);
			}

			void makeCurrLayerMoreVisible() {
				auto setOpacityImpl = [](auto& hiveTuple, uint8_t opacity) {
					detail::forEachDataMember([&](auto& objHive) {
						for (auto& editedObj : objHive) {
							editedObj.obj.setOpacity(opacity);
						}
						return detail::STAY_IN_LOOP;
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

			void showNodeOptions() {
				ImGui::SetNextWindowPos({ getNodeOptionsWindowPos(), });
				ImGui::SetNextWindowSize({ getSideWindowWidth(), getWindowHeight() });
				ImGui::Begin(NODE_OPTIONS_WINDOW_NAME);

				editNodeName();
				editLayerName();
				selectLayer();

				ImGui::End();
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
					detail::forEachDataMember([&](auto& objHive) {
						for (auto& obj : objHive) {
							obj.obj.screenMove(change);
						}
						return detail::STAY_IN_LOOP;
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
				auto change = toSDLFPoint(ImGui::GetMouseDragDelta());
				selectiveVisit([&](auto& selectedObject) {
					if (selectedObject.obj->obj.containsScreenCoord(mouse)) {
						selectedObject.obj->obj.screenMove(change);
						selectedObject.obj->obj.worldMove(change);
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
					detail::forEachDataMember([&, this](const auto& hive) {
						for (const auto& editedObj : hive) {
							editedObj.obj.render(renderer);
						}
						return detail::STAY_IN_LOOP;
					}, objects);
				}
			}

			void editSelectedObject(SDL_Renderer* renderer, Point mouse, ToolDisplay::GrabberTool& grabber) {
				grabber.render(renderer, mouse);
				selectObject(renderer, mouse, grabber);
				dragSelectedObject(mouse);
			}

			void runCurrentTool(SDL_Renderer* renderer, Point mouse, ToolDisplay& toolDisplay) {
				switch (toolDisplay.getCurrentTool()) {
				case Tool::Move:
					scroll();
					break;
				case Tool::Polygon:
					editPolygon(renderer, mouse);
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

			void showNodeWindow(SDL_Renderer* renderer, Point mouse) {
				configureNodeWindow();

				ImGui::Begin(OBJECT_WINDOW_NAME, nullptr, WINDOW_FLAGS);

				selectiveVisit([&, this](auto& selectedObj) {
					if (selectedObj.obj != nullptr) {
						edit(renderer, mouse, selectedObj);
					}
				}, m_selectedObject);

				ImGui::End();
			}
		public:
			NodeEditor(const std::string& name) : m_name{ name }, m_viewport{ getViewport(m_zoom) }
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

				for (const auto& layerJson : nodeJson[LAYERS_KEY]) {
					auto layerName = layerJson[NAME_KEY].get<std::string>();
					auto& currLayer = ret.m_layers.emplace_back(layerName);

					detail::forEachDataMember([&layerJson]<typename Object>(EditedObjectHive<Object>& objectGroup) {
						using BufferedObject = std::conditional_t<
							std::same_as<Object, DynamicPolygon>, BufferedPolygon, Object
						>;
						auto typeName = detail::getTypeName<BufferedObject>();
						auto objectGroupJsonIt = layerJson.find(typeName);
						
						if (objectGroupJsonIt == layerJson.end()) {
							return detail::STAY_IN_LOOP;
						}

						auto& objectGroupJson = *objectGroupJsonIt;
						for (const auto& objectJson : objectGroupJson) {
							objectGroup.insert(EditedObjectData<Object>::load(objectJson));
						}
						return detail::STAY_IN_LOOP;
					}, currLayer.objects);
				}

				return ret;
			}

			void show(SDL_Renderer* renderer, ToolDisplay& toolDisplay) {
				if (m_layers.empty()) {
					return;
				}

				renderSDLRect(renderer, m_viewport, { 0, 0, 0, 255 });
				auto viewportIntRect = toSDLRect(m_viewport);
				SDL_SetRenderViewport(renderer, &viewportIntRect);
				SDL_RenderClear(renderer);
				SDL_SetRenderScale(renderer, m_zoom, m_zoom);

				auto mouse = toSDLFPoint(ImGui::GetMousePos());

				showNodeWindow(renderer, mouse);
				zoom(renderer, mouse);
				render(renderer);
				if (windowContainsCoord(NODE_WINDOW_NAME, mouse)) {
					runCurrentTool(renderer, getViewportAdjustedMouse(renderer, mouse, m_zoom), toolDisplay);
				}
				showNodeOptions();

				SDL_SetRenderViewport(renderer, nullptr);
			}

			void addLayer(const std::string& layerName) {
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
			void transfer(EditedObjectData<Object>&& object) {
				auto& objects = std::get<EditedObjectHive<Object>>(m_layers[m_currLayerIdx].objects);
				auto insertedObjectIt = objects.insert(std::move(object));
				m_selectedObject = SelectedObjectData{ &(*insertedObjectIt), &objects, insertedObjectIt };
			}

			void deselectSelectedObject() noexcept {
				m_selectedObject = std::monostate{};
				m_draggingObject = false;
			}

			bool hasNoLayers() const noexcept {
				return m_layers.empty();
			}

			bool makingPolygon() const noexcept {
				return m_polygonBuilder.building();
			}

			const char* getName() const noexcept {
				return m_name.c_str();
			}
		private:
			static BufferedNode::TypeMap<size_t> calculateObjectRegionOffsets(const BufferedNode::TypeMap<size_t>& objectRegionLengths) {
				size_t currOffset = 0;
				BufferedNode::TypeMap<size_t> offsets{ 0 };
				objectRegionLengths.forEach([&]<typename Object>(size_t len) {
					currOffset = alignof(Object) + currOffset - 1;
					currOffset -= (currOffset % alignof(Object));

					offsets.get<Object>() = currOffset;
					currOffset += len;
				});
				return offsets;
			}

			template<bool IsBase = true, typename T>
			static constexpr void calculateSizeBytes(const T& t, BufferedNode::TypeMap<size_t>& objectRegionLengths) {
				if constexpr (concepts::Primitive<T>) {
					objectRegionLengths.get<T>() += sizeof(T);
				} else if constexpr (std::ranges::viewable_range<T>) {
					using ValueType = typename T::value_type;
					if constexpr (concepts::Primitive<ValueType>) {
						objectRegionLengths.get<ValueType>() += (sizeof(ValueType) * std::ranges::size(t));
					} else {
						for (const auto& elem : t) {
							calculateSizeBytes(elem, objectRegionLengths);
						}
					}
				} else {
					if constexpr (IsBase) {
						objectRegionLengths.get<T>() += sizeof(T);
					}
					detail::forEachDataMember([&]<typename Field>(const Field & field) {
						if constexpr (!concepts::Primitive<Field>) {
							calculateSizeBytes<false>(field, objectRegionLengths);
						}
						return detail::STAY_IN_LOOP;
					}, t);
				}
			}

			template<typename T>
			static decltype(auto) makeBufferedObject(const T& t) {
				if constexpr (std::same_as<T, DynamicPolygon>) {
					return detail::PolygonConverter::makeBufferedPolygon(t);
				} else {
					return (t);
				}
			}

			static void writeObjectData(json& currJsonLayer, const Layer::Objects& objects, BufferedNode::TypeMap<size_t>& objectRegionLengths) {
				detail::forEachDataMember([&]<typename Object>(const EditedObjectHive<Object>& hive) {
					using BufferedObject = std::remove_cvref_t<decltype(makeBufferedObject(std::declval<Object>()))>;

					auto typeName = detail::getTypeName<BufferedObject>();

					auto& objGroup = currJsonLayer[typeName] = json::array();
					for (const auto& obj : hive) {
						decltype(auto) bufferedObject = makeBufferedObject(obj.obj);

						if constexpr (std::same_as<nv::BufferedNode, BufferedObject>) {
							objectRegionLengths.get<std::byte*>() += obj.obj.getSizeBytes();
							objectRegionLengths.get<BufferedNode>() += sizeof(BufferedNode);
							objectRegionLengths.get<BufferedNode*>() += sizeof(BufferedNode*);
						} else {
							calculateSizeBytes(bufferedObject, objectRegionLengths);
						}

						objGroup.emplace_back() = obj;

						if (!obj.name.empty()) {
							using ObjectMapEntry = BufferedNode::ObjectMapEntry<BufferedObject>;
							objectRegionLengths.get<char>() += obj.name.size();
							objectRegionLengths.get<ObjectMapEntry>() += sizeof(ObjectMapEntry);
						}
					}

					return detail::STAY_IN_LOOP;
				}, objects);
			}

			static void writeLayerData(json& currJsonLayer, const std::string& layerName, BufferedNode::TypeMap<size_t>& objectRegionLengths) {
				if (!layerName.empty()) {
					currJsonLayer[NAME_KEY] = layerName;
					objectRegionLengths.get<char>() += layerName.size();
				}
			}

			std::string createSceneJson() {
				json root;
				root[NAME_KEY] = m_name;
				auto& layersRoot = root[LAYERS_KEY] = json::array();

				BufferedNode::TypeMap<size_t> objectRegionLengths{ 0 };
				objectRegionLengths.get<BufferedNode::Layer>() = m_layers.size() * sizeof(BufferedNode::Layer);

				for (const auto& [layerName, objects] : m_layers) {
					auto& currJsonLayer = layersRoot.emplace_back();

					if (!layerName.empty()) {
						currJsonLayer[NAME_KEY] = layerName;
						objectRegionLengths.get<char>() += layerName.size();
					}

					writeObjectData(currJsonLayer, objects, objectRegionLengths);
				}

				using BufferedNodeParser = nlohmann::adl_serializer<BufferedNode>;
				objectRegionLengths.forEach([&]<typename Object>(size_t size) {
					root[BufferedNodeParser::typeSizeKey<Object>()] = size;
				});

				auto offsets = calculateObjectRegionOffsets(objectRegionLengths);
				offsets.forEach([&]<typename Object>(size_t offset) {
					root[BufferedNodeParser::typeOffsetKey<Object>()] = offset;
				});

				root[BYTES_KEY] = offsets.getLast() + objectRegionLengths.getLast();
				return root.dump(2);
			}
		public:
			void saveAs() {
				try {
					saveNewFile({ { "node", "nv_node" } }, [this](const auto& path) {
						m_lastSavedFilePath = path;
						return createSceneJson();
					});
				}
				catch (json::exception e) {
					std::println("{}", e.what());
				}
			}

			void save() {
				if (m_lastSavedFilePath.empty()) {
					saveAs();
				}
				else {
					saveToExistingFile(m_lastSavedFilePath, createSceneJson());
				}
			}
		};
	}
}