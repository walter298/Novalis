#pragma warning(disable: 4005) //sdl_gfx defines M_PI for some reason

#include "App.h"

#include <concepts>
#include <chrono>
#include <stack>
#include <variant>

#include <boost/optional.hpp>
#include <boost/unordered/unordered_flat_set.hpp>
#include <magic_enum.hpp>

#include "../imgui/imgui.h"
#include "../imgui/imgui_impl_sdlrenderer3.h"
#include "../imgui/imgui_impl_sdl3.h"
#include "../imgui/imgui_internal.h"
#include "../imgui/imgui_stdlib.h"

#include "../data_util/Algorithms.h"
#include "../data_util/DataStructures.h"
#include "../data_util/File.h"
#include "../data_util/Reflection.h"
#include "../data_util/TypeErasure.h"
#include "../Collision.h"
#include "../Instance.h"

namespace {
	using namespace nv;
	
	struct SpecialPoint {
		static constexpr float WIDTH = 15.0f;
		static constexpr SDL_Color COLOR{ 0, 255, 0, 255 };

		std::string name;
		SDL_FPoint point{};

		inline void render(SDL_Renderer* renderer) const noexcept {
			SDL_FRect rect{ point.x, point.y, WIDTH, WIDTH };
			renderSDLRect(renderer, rect, COLOR);
		}

		bool containsCoord(SDL_FPoint p) const noexcept {
			SDL_FRect rect{ point.x, point.y, WIDTH, WIDTH };
			return SDL_PointInRectFloat(&p, &rect);
		}
	};

	template<typename Object>
	struct EditedObjectData {
		Object obj;
		float scale = 0;
		float width = 0;
		float height = 0;
		float x = 0;
		float y = 0;
		double angle = 0;
		uint8_t opacity = 255;
		SDL_FPoint rotationPoint{ 0, 0 };
		std::string name{ "name" };

		template<typename... Args>
		constexpr EditedObjectData(Args&&... args) requires(std::constructible_from<Object, Args...>)
			: obj{ std::forward<Args>(args)... }
		{
			if constexpr (concepts::SizeableObject<Object>) {
				auto size = obj.getSize();
				width = size.x;
				height = size.y;
			}
		}
	};

	SDL_FPoint toSDLFPoint(ImVec2 p) {
		return { p.x, p.y };
	}

	SDL_FRect getWindowRect(const char* windowName) {
		auto win = ImGui::FindWindowByName(windowName);
		auto pos = win->Pos;
		auto size = win->Size;
		return {
			pos.x,  pos.y,
			size.x, size.y
		};
	}

	template<typename T>
	void to_json(json& j, const EditedObjectData<T>& editedObj) {
		j = editedObj.obj;
	}

	template<typename Object>
	using EditedObjectHive = plf::hive<EditedObjectData<Object>>;

	constexpr const char* TOOL_WINDOW_NAME = "Tools";
	constexpr const char* OBJECT_WINDOW_NAME = "Object";
	constexpr const char* NODE_WINDOW_NAME = "Nodes";
	constexpr const char* NODE_OPTIONS_WINDOW_NAME = "Current Node";
	constexpr ImGuiWindowFlags WINDOW_FLAGS = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse;

	float getSideWindowWidth() {
		return ImGui::GetIO().DisplaySize.x * 0.2f;
	}

	float getWindowY() {
		return ImGui::GetIO().DisplaySize.y * 0.1f;
	}

	float getWindowHeight() {
		return ImGui::GetIO().DisplaySize.y - getWindowY();
	}

	ImVec2 getToolSize() {
		return {
			getSideWindowWidth() * 0.26f,
			getSideWindowWidth() * 0.26f
		};
	}

	float getToolWindowHeight() {
		return getWindowHeight() * 0.2f;
	}

	float getInputWidth() {
		return ImGui::GetIO().DisplaySize.x * 0.1f;
	}

	SDL_FPoint getViewportAdjustedMouse(SDL_Renderer* renderer) {
		auto mouse = toSDLFPoint(ImGui::GetMousePos());
		
		SDL_Rect viewport;
		SDL_GetRenderViewport(renderer, &viewport);
		mouse.x -= viewport.x;
		mouse.y -= viewport.y;

		return mouse;
	}

	ImVec2 getAdjacentWindowPos(const char* adjacentWindowName) {
		auto leftWindow = ImGui::FindWindowByName(adjacentWindowName);
		auto leftWindowPos = leftWindow->Pos;
		auto leftWindowSize = leftWindow->Size;
		return ImVec2{ (leftWindowPos.x + leftWindowSize.x), leftWindowPos.y };
	}

	ImVec2 getTabWindowPos() noexcept {
		return getAdjacentWindowPos(TOOL_WINDOW_NAME);
	}

	ImVec2 getTabWindowSize() {
		auto parentWindowSize = ImGui::GetIO().DisplaySize;
		return {
			parentWindowSize.x - (2.0f * getSideWindowWidth()),
			getWindowHeight()
		};
	}

	ImVec2 getNodeOptionsWindowPos() noexcept {
		return getAdjacentWindowPos(NODE_WINDOW_NAME);
	}

	SDL_FRect getViewport(float zoom) noexcept {
		auto tabWindowPos = getTabWindowPos();
		auto tabWindowSize = getTabWindowSize();
		return { 
			tabWindowPos.x / zoom, tabWindowPos.y / zoom, 
			tabWindowSize.x / zoom, tabWindowSize.y / zoom 
		};
	}

	enum class Tool {
		Move,
		ObjectSelect,
		AreaSelect,
		Polygon,
		Text,
		Delete
	};

	SDL_FPoint toSDLFPoint(BGPoint p) {
		return { p.get<0>(), p.get<1>() };
	}
	SDL_Point toSDLPoint(ImVec2 p) {
		return { static_cast<int>(p.x), static_cast<int>(p.y) };
	}

	class NodeEditor {
	private:
		float m_zoom = 1.0f;
		SDL_FRect m_viewport;
		std::string m_name;

		class PolygonBuilder {
		private:
			RenderPolygon m_polygon;
			bool m_placingNewPoint = false;

			SpecialPoint m_firstPoint;
			SpecialPoint m_lastPlacedPoint;
			bool m_building = false;
		public:
			bool building() const noexcept {
				return m_building;
			}

			std::optional<RenderPolygon> operator()(SDL_Renderer* renderer, SDL_FPoint mouse, float renderScaleW, float renderScaleH) noexcept {
				auto nodeWindowRect = getWindowRect(NODE_WINDOW_NAME);
				if (!SDL_PointInRectFloat(&mouse, &nodeWindowRect)) {
					return std::nullopt;
				}

				if (!m_building) {
					m_polygon.clear();
				}

				SDL_Rect viewport;
				SDL_GetRenderViewport(renderer, &viewport);
				mouse.x -= viewport.x;
				mouse.y -= viewport.y;
				mouse.x /= renderScaleW;
				mouse.y /= renderScaleH;

				ImGui::SetMouseCursor(ImGuiMouseCursor_None);
				m_lastPlacedPoint.point = mouse;
				m_lastPlacedPoint.render(renderer);

				if (!m_polygon.isEmpty()) {
					m_firstPoint.point = toSDLFPoint(m_polygon.getScreenCoord(0));
					m_firstPoint.render(renderer);
					m_polygon.render(renderer);
				}

				if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
					m_building = true;
					if (m_firstPoint.containsCoord(mouse) && m_polygon.getSize() > 1) {
						auto temp = m_polygon;
						m_polygon.clear();
						m_building = false;
						return temp;
					} else {
						m_polygon.add(mouse); //add a new point
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
			SelectedObjectData<RenderPolygon>, 
			//SelectedObjectData<Text>,
			//SelectedObjectData<Sprite>, 
			SelectedObjectData<Texture> 
			//SelectedObjectData<Rect>
			//SelectedObjectData<Node>
		>;
		SelectedObjectVariant m_selectedObject = std::monostate{};
		
		bool m_draggingObject = true;

		struct Layer {
			std::string name;
			std::tuple<
				EditedObjectHive<RenderPolygon>,
				//EditedObjectHive<Text>,
				//EditedObjectHive<Sprite>,
				EditedObjectHive<Texture>
				//EditedObjectHive<Rect>
				//EditedObjectHive<Node>
			> objects;
		};
		std::vector<Layer> m_layers;
		size_t m_currLayerIdx = 0;

		int m_worldX = 0;
		int m_worldY = 0;
		std::string m_worldXLabel;
		std::string m_worldYLabel;
		
		template<typename Object>
		void moveObjectByMouseDragDelta(SelectedObjectData<Object>& editedObj, SDL_FPoint mousePos) {
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
				m_selectedObject = std::monostate{};
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
			auto oldScale = editedObj.obj->scale;
			if (ImGui::SliderFloat("Scale", &editedObj.obj->scale, 0.0f, 1500.0f)) {
				auto deltaScale = editedObj.obj->scale - oldScale;
				editedObj.obj->obj.scale({ deltaScale, deltaScale });
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
				SDL_FPoint change{ editedObj.obj->x - wx, editedObj.obj->y - wy };
				editedObj.obj->obj.move(change);
			}
		}

		template<concepts::RenderableObject Object>
		void edit(SDL_Renderer* renderer, SDL_FPoint mousePos, SelectedObjectData<Object>& editedObj) {
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

		template<typename Object>
		bool selectObjectImpl(SDL_FPoint mouse, EditedObjectHive<Object>& objs) {
			//find first object that is clicked on
			/*auto selectedObjIt = std::ranges::find_if(objs, [&](const auto& editedObjData) -> bool {
				return editedObjData.obj.containsCoord(mouse);
			});
			if (selectedObjIt == objs.end()) {
				return false;
			}
			m_selectedObject = SelectedObjectData{
				&(*selectedObjIt), &objs, selectedObjIt
			};*/

			return true;
		}

		void selectObject(SDL_FPoint mouse) {
			if (ImGui::IsMouseReleased(ImGuiMouseButton_Left)) {
				m_draggingObject = false;
				return;
				/*if we have not released the button and we are dragging an object,
				then we don't need to select a new object*/
			} else if (!ImGui::IsMouseDown(ImGuiMouseButton_Left) || m_draggingObject) {
				return;
			}

			auto& objects = m_layers[m_currLayerIdx].objects;
			forEachDataMember([&, this](auto& hive) {
				if (selectObjectImpl(mouse, hive)) {
					m_draggingObject = true;
					return BREAK_FROM_LOOP;
				} else {
					return STAY_IN_LOOP;
				}
			}, objects);
		}

		void makeCurrLayerMoreVisible() {
			auto setOpacityImpl = [](auto& hiveTuple, uint8_t opacity) {
				forEachDataMember([&](auto& objHive) {
					for (auto& editedObj : objHive) {
						editedObj.obj.setOpacity(opacity);
					}
					return nv::STAY_IN_LOOP;
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

		void zoom(SDL_Renderer* renderer, SDL_FPoint mouse) {
			auto& io = ImGui::GetIO();
			if (ImGui::IsKeyDown(ImGuiKey_LeftCtrl) && io.MouseWheel != 0.0f) {
				auto dZoom = io.MouseWheel / 10.0f;
				m_zoom += dZoom;
				m_viewport = getViewport(m_zoom);
			}
		}

		void scroll() {
			auto change = toSDLFPoint(ImGui::GetMouseDragDelta());
			for (auto& [name, objects] : m_layers) {
				forEachDataMember([&](auto& objHive) {
					for (auto& obj : objHive) {
						obj.obj.screenMove(change);
					}
					return STAY_IN_LOOP;
				}, objects);
			}
			ImGui::ResetMouseDragDelta();
		}

		void dragSelectedObject() {
			auto change = toSDLFPoint(ImGui::GetMouseDragDelta());
			/*selectiveVisit([&](auto& selectedObject) {
				selectedObject.obj->obj.screenMove(change);
				selectedObject.obj->obj.worldMove(change);
			}, m_selectedObject);*/
		}

		void drag() {
			if (std::holds_alternative<std::monostate>(m_selectedObject)) {
				scroll();
			} else {
				dragSelectedObject();
			}
		}

		void editPolygon(SDL_Renderer* renderer, SDL_FPoint mouse) {
			auto polygon = m_polygonBuilder(renderer, mouse, m_zoom, m_zoom);
			if (polygon) {
				auto& polygons = std::get<EditedObjectHive<RenderPolygon>>(m_layers[m_currLayerIdx].objects);
				auto insertedPolygonIt = polygons.insert(std::move(*polygon));
				m_selectedObject = SelectedObjectData{ &(*insertedPolygonIt), &polygons, insertedPolygonIt };
			}
		}

		void render(SDL_Renderer* renderer) {
			for (auto& [layerName, objects] : m_layers) {
				forEachDataMember([&, this](const auto& hive) {
					for (const auto& editedObj : hive) {
						editedObj.obj.render(renderer);
					}
					return STAY_IN_LOOP;
				}, objects);
			}
			//SDL_SetRenderViewport(renderer, nullptr);
		}

		void runCurrentTool(SDL_Renderer* renderer, SDL_FPoint mouse, Tool currTool) {
			switch (currTool) {
			case Tool::Move:
				drag();
				break;
			case Tool::Polygon:
				editPolygon(renderer, mouse);
				break;
			case Tool::ObjectSelect:
				selectObject(mouse);
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

		void showNodeWindow(SDL_Renderer* renderer, SDL_FPoint mouse) {
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
		NodeEditor(const char* name) : m_name{ name }, m_viewport{ getViewport(m_zoom) }
		{
		}

		void show(SDL_Renderer* renderer, Tool currTool) {
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
			scroll();
			zoom(renderer, mouse);
			render(renderer);
			runCurrentTool(renderer, mouse, currTool);
			showNodeOptions();

			SDL_SetRenderViewport(renderer, nullptr);
		}

		void addLayer(const std::string& layerName) {
			if (m_layers.empty()) {
				m_layers.emplace_back(layerName);
			} else {
				//insert layer after the current layer
				m_layers.emplace(m_layers.begin() + m_currLayerIdx + 1, layerName);
				m_currLayerIdx++;
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
			std::get<EditedObjectHive<Object>>(m_layers[m_currLayerIdx].objects).insert(
				std::move(object)
			);
		}

		void deselectSelectedObject() noexcept {
			m_selectedObject = std::monostate{};
			m_draggingObject = false;
		}

		bool empty() const noexcept {
			return m_layers.empty();
		}

		bool makingPolygon() const noexcept {
			return m_polygonBuilder.building();
		}

		const char* getName() const noexcept {
			return m_name.c_str();
		}
	};

	class NodeTabList {
	private:
		plf::hive<NodeEditor> m_tabs;
		NodeEditor* m_currTab = nullptr;
		
		ImVec2 getTabWindowPos() noexcept {
			auto leftWindow = ImGui::FindWindowByName(TOOL_WINDOW_NAME);
			auto leftWindowPos = leftWindow->Pos;
			auto leftWindowSize = leftWindow->Size;
			return ImVec2{ (leftWindowPos.x + leftWindowSize.x), leftWindowPos.y };
		}

		ImVec2 getTabWindowSize() {
			auto parentWindowSize = ImGui::GetIO().DisplaySize;
			return {
				parentWindowSize.x - (2.0f * getSideWindowWidth()),
				getWindowHeight()
			};
		}

		void showTabBar() {
			auto tabWindowPos = getTabWindowPos();
			auto tabWindowSize = getTabWindowSize();

			ImGui::SetNextWindowPos(tabWindowPos);
			ImGui::SetNextWindowSize(tabWindowSize);

			ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4{ 0.0f, 0.0f, 0.0f, 0.f });

			ImGui::Begin(NODE_WINDOW_NAME, nullptr, WINDOW_FLAGS);

			bool deletedTab = false;

			if (ImGui::BeginTabBar("Tabs")) {
				for (auto it = m_tabs.begin(); it != m_tabs.end(); it++) {
					auto& tab = *it;

					if (ImGui::BeginTabItem(tab.getName())) {
						m_currTab = &tab;
						ImGui::Text(tab.getName());
						ImGui::EndTabItem();
					}
				}
				ImGui::EndTabBar();
			}

			ImGui::PopStyleColor();

			ImGui::End();
		}
	public:
		void show(SDL_Renderer* renderer, Tool currTool) noexcept {
			showTabBar();
			if (m_currTab != nullptr) {
				m_currTab->show(renderer, currTool);
			}
		}
 
		void add(const std::string& name) {
			m_tabs.emplace(name.c_str());
		}

		boost::optional<NodeEditor&> currentTab() {
			if (m_currTab) {
				return *m_currTab;
			} else {
				return boost::none;
			}
		}

		bool empty() const noexcept {
			return m_tabs.empty();
		}

		bool makingPolygon() const noexcept {
			return m_currTab ? m_currTab->makingPolygon() : false;
		}
	};

	static std::optional<std::vector<EditedObjectData<Texture>>> createTexturesFromImages(SDL_Renderer* renderer) {
		auto texPaths = openMultipleFiles({ { "images", "png" } });
		if (!texPaths) {
			return std::nullopt;
		}

		std::vector<EditedObjectData<Texture>> textures;

		for (const auto& texPath : *texPaths) {
			auto& editedTex = textures.emplace_back(
				renderer, texPath.c_str()
			);
			editedTex.name = fileName(texPath);
			editedTex.x = editedTex.obj.getScreenPos().x;
			editedTex.y = editedTex.obj.getScreenPos().y;
		}

		return textures;
	}

	void centerNextText(const char* text) {
		auto windowWidth = ImGui::GetWindowSize().x;
		auto textWidth = ImGui::CalcTextSize(text).x;
		ImGui::SetCursorPosX((windowWidth - textWidth) * 0.5f);
	}

	void showDisabledMenu(const char* text) {
		ImGui::BeginDisabled();
		if (ImGui::BeginMenu(text)) {
			ImGui::EndMenu();
		}
		ImGui::EndDisabled();
	}

	enum class ObjectCreationOption {
		None,
		Text,
		Rect,
		Polygon,
		Texture,
		Background
	};

	template<concepts::MoveableObject Object>
	void setGridLayout(std::vector<EditedObjectData<Object>>& objects) {
		auto maxHeight = ranges::max(objects, [](const auto& a, const auto& b) {
			return a.obj.getWorldSize().y > b.obj.getWorldSize().y;
		}).obj.getWorldSize().x;

		int columnCount = static_cast<int>(objects.size());
		float x = 0.0;
		float y = 0.0;
		for (auto& obj : objects) {
			obj.obj.setScreenPos({ x, y });
			if (x + 1 == columnCount) {
				x = 0;
				y += maxHeight;
			} else {
				x += obj.obj.getScreenSize().x;
			}
		}
	}

	template<ranges::viewable_range Objects>
	bool showLayoutOption(Objects objects) {
		enum class ObjectLayout {
			Grid,
			Stack,
			Row,
			None
		};
		static auto currentlyChosenLayout = ObjectLayout::None;
		static std::string layoutName = "No layout chosen";
		
		ImGui::Begin("Choose Layout");
		ScopeExit imGuiEnd{ [] { ImGui::End(); } };

		if (objects.size() % 2 == 0 && ImGui::Button("Grid")) {
			currentlyChosenLayout = ObjectLayout::Grid;
			layoutName = "Current layout: grid";
			//setGridLayout(objects);
		}
		if (ImGui::Button("Stack")) {
			currentlyChosenLayout = ObjectLayout::Stack;
			layoutName = "Current layout: stack";
		}
		if (ImGui::Button("Row")) {
			currentlyChosenLayout = ObjectLayout::Row;
			layoutName = "Current layout: row";
		}

		ImGui::Text(layoutName.c_str());

		ImGui::SetNextItemWidth(getInputWidth());
		ImGui::BeginDisabled(currentlyChosenLayout == ObjectLayout::None);
		ScopeExit endDisabled{ [] { ImGui::EndDisabled(); } };

		if (ImGui::Button("Set")) {
			currentlyChosenLayout = ObjectLayout::None;
			layoutName = "No layout chosen";
			return true;
		}
		
		return false;
	}

	void showObjectDropdown(SDL_Renderer* renderer, NodeTabList& tabs) {
		static std::variant<
			std::vector<EditedObjectData<Texture>>
			//std::vector<EditedObjectData<Node>>
		> currentlyLoadedObjects;

		static bool showingLayoutPopup = false;
		if (showingLayoutPopup) {
			std::visit([&](auto& objects) {
				if (showLayoutOption(objects)) {
					tabs.currentTab()->transfer(objects);
					showingLayoutPopup = false;
				}
			}, currentlyLoadedObjects);
		}

		if (!tabs.currentTab() || tabs.currentTab()->empty()) {
			showDisabledMenu("Object");
			return;
		}

		auto addObjects = [&](auto objects) {
			if (objects) {
				if (objects->size() > 1) {
					currentlyLoadedObjects = *objects;
					showingLayoutPopup = true;
				} else {
					assert(!objects->empty());
					auto& object = (*objects)[0];
					tabs.currentTab()->transfer(std::move(object));
				}
			}
		};

		if (ImGui::BeginMenu("Object")) {
			if (ImGui::MenuItem("Create Textures From Images")) {
				addObjects(createTexturesFromImages(renderer));
			}
			ImGui::Separator();
			if (ImGui::MenuItem("Create Text")) {

			}
			ImGui::Separator();
			if (ImGui::MenuItem("Create Polygon")) {

			}
			ImGui::Separator();
			if (ImGui::MenuItem("Upload Node")) {

			}
			ImGui::EndMenu();
		}
	}

	constexpr const char* LAYER_CREATION_POPUP_NAME = "Create Layer";

	void showLayerCreationPopupWindow(NodeTabList& tabs, bool& showingAddNewLayerPopup) {
		auto center = ImGui::GetMainViewport()->GetCenter();
		ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2{ 0.5f, 0.5f });

		ImGui::OpenPopup(LAYER_CREATION_POPUP_NAME);
		ImGui::BeginPopupContextWindow(LAYER_CREATION_POPUP_NAME);

		centerNextText("Create Layer");
		ImGui::Text("Create Layer");

		static std::string layerName;
		ImGui::InputText("Layer Name", &layerName);

		centerNextText("Create");
		if (ImGui::Button("Create")) {
			auto& currTab = *tabs.currentTab();
			currTab.addLayer(layerName);
			showingAddNewLayerPopup = false;
			ImGui::CloseCurrentPopup();
		}

		ImGui::EndPopup();
	}

	void showLayerDropdown(NodeTabList& tabs) {
		if (tabs.empty()) {
			showDisabledMenu("Layer");
			return;
		}

		static bool showingAddNewLayerPopup = false;
		if (showingAddNewLayerPopup) {
			showLayerCreationPopupWindow(tabs, showingAddNewLayerPopup);
		}

		if (ImGui::BeginMenu("Layer")) {
			if (ImGui::MenuItem("New Layer")) {
				showingAddNewLayerPopup = true;
			}
			if (ImGui::MenuItem("Delete Layer")) {

			}
			if (ImGui::MenuItem("Duplicate Layer")) {

			}
			ImGui::EndMenu();
		}
	}

	constexpr const char* NODE_CREATION_POPUP_NAME = "Create Node";

	void showNodeCreationPopupWindow(NodeTabList& tabs, bool& showingNodeCreationPopupWindow) {
		auto center = ImGui::GetMainViewport()->GetCenter();
		ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
		
		ImGui::OpenPopup(NODE_CREATION_POPUP_NAME);
		ImGui::BeginPopupContextWindow(NODE_CREATION_POPUP_NAME);

		centerNextText("Create Node");
		ImGui::Text("Create Node");

		static std::string nodeNameInput;
		ImGui::InputText("Node Name", &nodeNameInput);

		centerNextText("Create");
		if (ImGui::Button("Create")) {
			showingNodeCreationPopupWindow = false;
			tabs.add(nodeNameInput);
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}

	void showFileDropdown(NodeTabList& tabs) noexcept {
		static bool showingNodeCreationPopupWindow = false;
		if (showingNodeCreationPopupWindow) {
			showNodeCreationPopupWindow(tabs, showingNodeCreationPopupWindow);
		}

		if (ImGui::BeginMenu("File")) {
			if (ImGui::MenuItem("New Node")) {
				showingNodeCreationPopupWindow = true;
			}
			if (ImGui::MenuItem("Open Node")) {

			}
			ImGui::Separator();
			if (ImGui::MenuItem("Save")) {

			}
			if (ImGui::MenuItem("Save as")) {

			}
			ImGui::EndMenu();
 		}
	}

	void showTopButtonRow(SDL_Renderer* renderer, NodeTabList& tabs) {
		ImGui::BeginMainMenuBar();
		showFileDropdown(tabs);
		showLayerDropdown(tabs);
		showObjectDropdown(renderer, tabs);
		ImGui::EndMainMenuBar();
	};

	class ToolButtonList {
		struct ToolButtonData {
			Tool tool = Tool::Move;
			TexturePtr tex;
		};

		std::vector<ToolButtonData> m_buttons;
		Tool m_currTool = Tool::Move;
	public:
		ToolButtonList(SDL_Renderer* renderer) {
			namespace fs = std::filesystem;
			for (const auto& filePath : fs::directory_iterator(workingDirectory() + "novalis_assets/tool_images")) {
				if (filePath.is_directory()) {
					continue;
				}
				auto filename = filePath.path().stem().string();
				auto tool = magic_enum::enum_cast<Tool>(filename);
				assert(tool.has_value());
				auto pathStr = filePath.path().string();
				m_buttons.emplace_back(tool.value(), TexturePtr{ renderer, pathStr.c_str() });
			}
		}

		void show(bool makingPolygon) noexcept {
			ImGui::SetNextWindowPos({ 0.0f, getWindowY() });
			ImGui::SetNextWindowSize({ getSideWindowWidth(), getWindowHeight() * 0.3f });
			
			ImGui::BeginDisabled(makingPolygon);
			ImGui::Begin(TOOL_WINDOW_NAME, nullptr, WINDOW_FLAGS);

			int i = 1;
			for (const auto& [tool, tex] : m_buttons) {
				const bool highlighted = tool == m_currTool;
				if (highlighted) {
					ImGui::PushStyleColor(ImGuiCol_Button, { 0, 255, 0, 255 });
					ImGui::PushStyleColor(ImGuiCol_ButtonHovered, { 0, 255, 0, 255 });
					ImGui::PushStyleColor(ImGuiCol_ButtonActive, { 0, 255, 0, 255 });
				}
				auto strId = std::to_string(i);
				if (ImGui::ImageButton(strId.c_str(), reinterpret_cast<ImTextureID>(tex.tex), getToolSize())) {
					m_currTool = tool;
				}
				if (highlighted) {
					ImGui::PopStyleColor();
					ImGui::PopStyleColor();
					ImGui::PopStyleColor();
				}
				if (i % 3 != 0) {
					ImGui::SameLine();
				}
				i++;
			}
			ImGui::End();
			ImGui::EndDisabled();
		}

		Tool currentTool() const noexcept {
			return m_currTool;
		}
	};

	struct AppData {
		nv::Instance instance{ "Novalis" };
		NodeTabList tabs;
		ToolButtonList tools{ instance.renderer };
		bool running = true;
	};
}

void nv::editor::runApp() {
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	
	AppData app;

	auto& io = ImGui::GetIO();
	(void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; 
	ImGui::StyleColorsDark();
	ImGui_ImplSDL3_InitForSDLRenderer(app.instance.window, app.instance.renderer);
	ImGui_ImplSDLRenderer3_Init(app.instance.renderer);

	while (app.running) {
		constexpr float FONT_SCALE = 2.1f;
		ImGui::GetIO().FontGlobalScale = FONT_SCALE;

		using namespace std::literals;

		constexpr auto waitTime = 1000ms / 500;
		const auto endTime = std::chrono::system_clock::now() + waitTime;

		SDL_Event evt;
		while (SDL_PollEvent(&evt)) {
			ImGui_ImplSDL3_ProcessEvent(&evt);
			if (evt.type == SDL_EVENT_QUIT) {
				app.running = false;
			}
		}

		SDL_RenderClear(app.instance.renderer);
		
		static constexpr ImVec4 color{ 0.45f, 0.55f, 0.60f, 1.00f };

		ImGui_ImplSDLRenderer3_NewFrame();
		ImGui_ImplSDL3_NewFrame();
		ImGui::NewFrame();

		showTopButtonRow(app.instance.renderer, app.tabs);

		app.tools.show(app.tabs.makingPolygon());
		app.tabs.show(app.instance.renderer, app.tools.currentTool());

		const auto now = std::chrono::system_clock::now();
		if (now < endTime) {
			std::this_thread::sleep_for(endTime - now);
		}

		SDL_SetRenderDrawColor(app.instance.renderer,
			static_cast<uint8_t>(color.x * 255), static_cast<uint8_t>(color.y * 255),
			static_cast<uint8_t>(color.z * 255), static_cast<uint8_t>(color.w * 255));
		SDL_SetRenderScale(app.instance.renderer, io.DisplayFramebufferScale.x, io.DisplayFramebufferScale.y);
		ImGui::Render();
		ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), app.instance.renderer);
		SDL_RenderPresent(app.instance.renderer);
	}

	ImGui_ImplSDLRenderer3_Shutdown();
	ImGui_ImplSDL3_Shutdown();
	ImGui::DestroyContext();
}