#include <novalis/detail/file/File.h>
#include <novalis/detail/reflection/TemplateDetection.h>

#include "BufferedAlias.h"
#include "ErrorPopup.h"
#include "NodeEditor.h"
#include "PolygonOutline.h"
#include "ToolDisplay.h"

//for static methods
using namespace nv;
using namespace editor;

template<typename Callable, typename Variant>
static void selectiveVisit(Callable callable, Variant& variant) {
	std::visit([&callable]<typename Object>(Object& visited) {
		if constexpr (!std::same_as<std::monostate, Object>) {
			std::invoke(callable, visited);
		}
	}, variant);
}

void nv::editor::NodeEditor::selectObject(SDL_Renderer* renderer, Point mouse) {
	if (!ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
		return;
	}

	nv::detail::forEachDataMember([&, this]<typename Object>(EditedObjectHive<Object>&currObjectHive) {
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
			}
			else {
				editedObjData.obj.setOpacity(100);
			}
		}
		return nv::detail::STAY_IN_LOOP;
	}, m_layers[m_currLayerIdx].objects);
}

void nv::editor::NodeEditor::makeCurrLayerMoreVisible() {
	auto setOpacityImpl = [](auto& hiveTuple, uint8_t opacity) {
		nv::detail::forEachDataMember([&](auto& objHive) {
			for (auto& editedObj : objHive) {
				editedObj.obj.setOpacity(opacity);
			}
			return nv::detail::STAY_IN_LOOP;
			}, hiveTuple);
		};

	auto setOpacity = [&, this](auto layers) {
		for (auto& [layerName, objects] : layers) {
			setOpacityImpl(objects, m_externalLayerOpacity);
		}
	};

	//reduce opacity of all layers before the current layer
	setOpacity(ranges::subrange(m_layers.begin(), m_layers.begin() + m_currLayerIdx));

	//reduce opacity of all layers after the current layer
	setOpacity(ranges::subrange(m_layers.begin() + m_currLayerIdx + 1, m_layers.end()));

	//make the current layer visible in case that its opacity was reduced
	setOpacityImpl(m_layers[m_currLayerIdx].objects, 255);
}

void nv::editor::NodeEditor::editLayerName() {
	ImGui::SetNextItemWidth(getInputWidth());
	auto& currLayerName = m_layers[m_currLayerIdx].name;
	m_layerNameManager.inputName("Layer name", currLayerName);
}

void nv::editor::NodeEditor::selectLayer() {
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

void nv::editor::NodeEditor::showObjectFilterOptions() {
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
	showSelectionOption("Spritesheets", std::type_identity<Spritesheet>{});
}

void nv::editor::NodeEditor::showExternalLayerOpacityOption() {
	ImGui::SetNextItemWidth(getInputWidth());
	auto temp = static_cast<int>(m_externalLayerOpacity);
	if (ImGui::SliderInt("External Layer Opacity", &temp, 0, 255)) {
		m_externalLayerOpacity = static_cast<uint8_t>(temp);
		makeCurrLayerMoreVisible();
	}
}

void nv::editor::NodeEditor::showNodeOptions(bool disabled) {
	ImGui::BeginDisabled(disabled);

	ImGui::SetNextWindowPos(getNodeOptionsWindowPos());
	ImGui::SetNextWindowSize(getNodeOptionsWindowSize());
	ImGui::Begin(NODE_OPTIONS_WINDOW_NAME);

	showObjectFilterOptions();
	editLayerName();
	selectLayer();
	showExternalLayerOpacityOption();
	m_objectGroupManager.showAllObjectGroups(m_objectGroupNameManager);

	ImGui::End();

	ImGui::EndDisabled();
}

void nv::editor::NodeEditor::zoom(SDL_Renderer* renderer, Point mouse) {
	auto& io = ImGui::GetIO();
	if (ImGui::IsKeyDown(ImGuiKey_LeftCtrl) && io.MouseWheel != 0.0f) {
		auto dZoom = io.MouseWheel / 10.0f;
		m_zoom += dZoom;
		m_viewport = getViewport(m_zoom);
	}
}

void nv::editor::NodeEditor::scroll() {
	auto change = toSDLFPoint(ImGui::GetMouseDragDelta());
	m_worldOffsetX += change.x;
	m_worldOffsetY += change.y;

	for (auto& [name, objects] : m_layers) {
		nv::detail::forEachDataMember([&](auto& objHive) {
			for (auto& obj : objHive) {
				obj.obj.move(change);
			}
			return nv::detail::STAY_IN_LOOP;
		}, objects);
	}
	ImGui::ResetMouseDragDelta();
}

void nv::editor::NodeEditor::dragSelectedObject(Point mouse) {
	Point change = toSDLFPoint(ImGui::GetMouseDragDelta());
	change /= m_zoom;
	selectiveVisit([&](auto& selectedObject) {
		if (selectedObject.obj->obj.containsScreenCoord(mouse) || ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
			m_objectGroupManager.move(*selectedObject.obj, change);
		}
	}, m_selectedObject);
	ImGui::ResetMouseDragDelta();
}

void nv::editor::NodeEditor::editPolygon(SDL_Renderer* renderer, Point mouse) {
	auto polygon = m_polygonBuilder(renderer, mouse, m_worldOffsetX, m_worldOffsetY);
	if (polygon) {
		polygon->screenScale(1.0f / m_zoom);
		polygon->worldScale(1.0f / m_zoom);
		auto& polygons = std::get<EditedObjectHive<DynamicPolygon>>(m_layers[m_currLayerIdx].objects);
		auto insertedPolygonIt = polygons.insert(std::move(*polygon));
		m_selectedObject = SelectedObjectData{ &(*insertedPolygonIt), &polygons, insertedPolygonIt };
	}
}

void nv::editor::NodeEditor::render(SDL_Renderer* renderer) const noexcept {
	for (auto& [layerName, objects] : m_layers) {
		nv::detail::forEachDataMember([&, this](const auto& hive) {
			for (const auto& editedObj : hive) {
				editedObj.obj.render(renderer);
			}
			return nv::detail::STAY_IN_LOOP;
		}, objects);
	}
}

void nv::editor::NodeEditor::editSelectedObject(SDL_Renderer* renderer, Point mouse) {
	auto renderAdjustedMouse = getViewportAdjustedMouse(renderer, mouse, m_zoom);
	selectObject(renderer, renderAdjustedMouse);
	dragSelectedObject(renderAdjustedMouse);
}

void nv::editor::NodeEditor::runCurrentTool(SDL_Renderer* renderer, Point mouse) {
	auto currTool = getCurrentTool();
	if (currTool == Tool::ObjectSelect && m_selectingObject) {
		makeCurrLayerMoreVisible();
	}
	switch (currTool) {
	case Tool::Move:
		scroll();
		break;
	case Tool::Polygon:
		editPolygon(renderer, getViewportAdjustedMouse(renderer, mouse, m_zoom));
		break;
	case Tool::ObjectSelect:
		editSelectedObject(renderer, mouse);
		break;
	}
}

static void moveObjectByMouseDragDelta(auto& editedObj, Point mousePos) {
	if (editedObj.obj->obj.containsCoord(mousePos)) {
		auto mouseChange = toSDLFPoint(ImGui::GetMouseDragDelta());
		editedObj.obj->obj.move(mouseChange);
		ImGui::ResetMouseDragDelta();
	}
}

//use auto because SelectedObjectData is a private class
template<typename Object>
static void showObjectRotationOption(ObjectMetadata<Object>& editedObj) {
	ImGui::SetNextItemWidth(getInputWidth());
	ImGui::Text("Rotation");
	ImGui::SetNextItemWidth(getInputWidth());
	if (ImGui::SliderFloat("Angle", &editedObj.angle, 0.0f, 360.0f)) {
		editedObj.obj.setRotation(editedObj.angle);
	}
	/*ImGui::SetNextItemWidth(getInputWidth());
	if (ImGui::InputInt("Rotation x", &editedObj.obj->rotationPoint.x)) {
		editedObj.obj->obj.rotate(static_cast<double>(floatAngle), editedObj.obj->rotationPoint);
	}
	ImGui::SetNextItemWidth(getInputWidth());
	if (ImGui::InputInt("Rotation y", &editedObj.obj->rotationPoint.y)) {
		editedObj.obj->obj.rotate(static_cast<double>(floatAngle), editedObj.obj->rotationPoint);
	}
	editedObj.obj->angle = static_cast<double>(floatAngle);*/
}

//use auto since SelectedObjectData is private
static void showObjectDuplicationOption(auto& editedObj, NameManager& nameManager) {
	ImGui::SetNextItemWidth(getInputWidth());
	if (ImGui::Button("Duplicate")) {
		assert(editedObj.objLayer);
		auto copiedObjectIt = editedObj.objLayer->insert(*editedObj.obj);
		copiedObjectIt->loadName(nameManager.makeUniqueName());
		editedObj.obj = &(*copiedObjectIt);
		editedObj.it = copiedObjectIt;
	}
}

template<typename Object>
static void showOpacityOption(ObjectMetadata<Object>& editedObj, ObjectGroupManager& objectGroupManager) {
	ImGui::SetNextItemWidth(getInputWidth());
	auto opacityInt = static_cast<int>(editedObj.opacity);
	if (ImGui::SliderInt("Opacity", &opacityInt, 0, 255)) {
		objectGroupManager.setOpacity(editedObj, static_cast<uint8_t>(opacityInt));
	}
}

template<concepts::ScaleableObject Object>
static void showScaleOption(ObjectMetadata<Object>& editedObj, ObjectGroupManager& objectGroupManager) {
	ImGui::SetNextItemWidth(getInputWidth());
	if (ImGui::SliderFloat("Scale", &editedObj.scale, 1.0f, 5.0f)) {
		objectGroupManager.scale(editedObj, editedObj.scale);
	}
}

static void editSpritesheet(Spritesheet& spritesheet) {
	auto rowIdx = spritesheet.getRowIndex();
	ImGui::SetNextItemWidth(getInputWidth());
	ImGui::SliderInt("Row Index", &rowIdx, 0, spritesheet.getRowCount() - 1);

	auto colIdx = spritesheet.getColumnIndex();
	ImGui::SetNextItemWidth(getInputWidth());
	ImGui::SliderInt("Column Index", &colIdx, 0, spritesheet.getColumnCount() - 1);

	spritesheet.setTextureIndex(rowIdx, colIdx);
}

static bool showObjectDeletionOption(auto& editedObj, ObjectGroupManager& objectGroupManager) {
	ImGui::SetNextItemWidth(getInputWidth());
	if (ImGui::Button("Delete")) {
		objectGroupManager.removeFromAllObjectGroups(*editedObj.obj);
		editedObj.obj->destroy();
		editedObj.objLayer->erase(editedObj.it);
		editedObj.obj = nullptr;
		return true;
	}
	return false;
}

void nv::editor::NodeEditor::createCollisionOutlines(SDL_Renderer* renderer, ID<EditedObjectGroup> groupID,
	ObjectMetadata<Texture>& editedTex)
{
	m_objectGroupManager.addObjectToGroup(groupID, editedTex);
	auto collisionOutlines = getPolygonOutlines(renderer, editedTex.obj);
	for (auto& outline : collisionOutlines) {
		auto& editedOutline = transfer(ObjectMetadata<DynamicPolygon>{ std::move(outline) });
		editedOutline.groupIDs.insert(groupID);
		m_objectGroupManager.addObjectToGroup(groupID, editedOutline);
	}
}

void nv::editor::NodeEditor::showCollisionOutlineOption(SDL_Renderer* renderer, ObjectMetadata<Texture>& editedTex) {
	ImGui::SetNextItemWidth(getInputWidth());
	if (ImGui::Button("Create collision outline")) {
		auto spriteGroupID = m_objectGroupManager.addGroup();
		createCollisionOutlines(renderer, spriteGroupID, editedTex);
	}
}

void nv::editor::NodeEditor::showObjectGroupCreationWindow() {
	if (!m_objectGroupCreator.show(m_objectGroupManager, m_objectGroupNameManager)) {
		m_creatingObjectGroup = false;
	}
}

template<typename Object>
void showFlipOption(ObjectMetadata<Object>& object) {
	ImGui::SetNextItemWidth(getInputWidth());
	if (ImGui::Button("Flip Horizontally")) {
		object.obj.flipHorizontally();
	}
	ImGui::SetNextItemWidth(getInputWidth());
	if (ImGui::Button("Flip Vertically")) {
		object.obj.flipVertically();
	}
}

void nv::editor::NodeEditor::showNodeWindow(SDL_Renderer* renderer, Point mouse) {
	ImGui::SetNextWindowPos(getChildNodeWindowPos());
	ImGui::SetNextWindowSize(getChildNodeWindowSize());
	
	ImGui::Begin(CHILD_NODE_WINDOW_NAME, nullptr, DEFAULT_WINDOW_FLAGS);

	selectiveVisit([&, this]<typename Object>(SelectedObjectData<Object>& selectedObject) {
		ImGui::BeginDisabled(isBusy());

		if constexpr (std::same_as<Object, Spritesheet>) {
			editSpritesheet(selectedObject.obj->obj);
		}
		//show opacity
		showOpacityOption(*selectedObject.obj, m_objectGroupManager);

		//scaling texture
		if constexpr (concepts::ScaleableObject<Object>) {
			showScaleOption(*selectedObject.obj, m_objectGroupManager);
		}

		//rotation
		if constexpr (concepts::RotateableObject<Object>) {
			showObjectRotationOption(*selectedObject.obj);
		}

		//name input
		selectedObject.obj->inputName(m_objectNameManager);

		//duplication 
		if constexpr (std::copy_constructible<Object>) {
			showObjectDuplicationOption(selectedObject, m_objectNameManager);
		}

		//collision outline
		if constexpr (std::same_as<Object, Texture>) {
			showCollisionOutlineOption(renderer, *selectedObject.obj);
		}

		//flip
		showFlipOption(*selectedObject.obj);

		//deletion
		if (showObjectDeletionOption(selectedObject, m_objectGroupManager)) {
			deselectSelectedObject();
		}

		ImGui::EndDisabled();
	}, m_selectedObject);

	ImGui::End();
}

std::optional<nv::editor::NodeEditor> nv::editor::NodeEditor::load(const nlohmann::json& nodeJson, FileID id, 
	ErrorPopup& errorPopup) noexcept
{
	try {
		NodeEditor ret{ id };

		auto& layersJson = nodeJson[LAYERS_KEY];
		ret.m_layers.reserve(nodeJson.size());

		for (const auto& layerJson : nodeJson[LAYERS_KEY]) {
			auto layerName = layerJson[NAME_KEY].get<std::string>();
			auto& currLayer = ret.m_layers.emplace_back(layerName);

			nv::detail::forEachDataMember([&layerJson]<typename Object>(EditedObjectHive<Object>&objectGroup) {
				auto typeName = nv::detail::getTypeName<BufferedObject<Object>>();
				auto objectGroupJsonIt = layerJson.find(typeName);

				if (objectGroupJsonIt == layerJson.end()) {
					return nv::detail::STAY_IN_LOOP;
				}

				auto& objectGroupJson = *objectGroupJsonIt;
				for (const auto& objectJson : objectGroupJson) {
					objectGroup.insert(ObjectMetadata<Object>::load(objectJson));
				}
				return nv::detail::STAY_IN_LOOP;
			}, currLayer.objects);
		}
		return ret;
	} catch (const nlohmann::json::exception& e) {
		errorPopup.add(std::format("Error parsing node: {}", e.what()));
	}
	return std::nullopt;
}

void nv::editor::NodeEditor::updateNode(FileID fileID, BufferedNode node) {
	auto& nodeData = *m_children.at(fileID);
	nodeData.obj = std::move(node);
}

void nv::editor::NodeEditor::show(SDL_Renderer* renderer) {
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

	viewportIntRect = toSDLRect(getViewport(1.0f));
	SDL_SetRenderViewport(renderer, &viewportIntRect);
	SDL_SetRenderScale(renderer, 1.0f, 1.0f);

	if (windowContainsCoord(TAB_WINDOW_NAME, mouse)) {
		runCurrentTool(renderer, mouse);
	}

	if (m_creatingObjectGroup) {
		showObjectGroupCreationWindow();
	}
	showNodeOptions(m_creatingObjectGroup);

	SDL_SetRenderViewport(renderer, nullptr);
}

void nv::editor::NodeEditor::addLayer(std::string layerName) {
	m_layerNameManager.makeExistingNameUnique(layerName);

	m_selectedObject = std::monostate{};

	if (m_layers.empty()) {
		m_layers.emplace_back(layerName);
	} else {
		m_layers.emplace(m_layers.begin() + m_currLayerIdx + 1, layerName);
	}
}

NodeSerializationResult nv::editor::NodeEditor::serialize() const {
	return createNodeJson(m_layers, m_objectGroupManager);
}

ObjectMetadata<BufferedNode>& nv::editor::NodeEditor::transfer(FileID childID, ObjectMetadata<BufferedNode>&& node) {
	auto& nodeRef = transferImpl(std::move(node));
	m_children.emplace(childID, &nodeRef);
	return nodeRef;
}

void nv::editor::NodeEditor::createObjectGroup() {
	m_creatingObjectGroup = true;
	m_objectGroupCreator.setNewObjects(m_layers);
	ImGui::OpenPopup(OBJECT_GROUP_CREATION_WINDOW_NAME);
}

void nv::editor::NodeEditor::deselectSelectedObject() noexcept {
	m_selectedObject = std::monostate{};
	m_draggingObject = false;
}

bool nv::editor::NodeEditor::hasNoLayers() const noexcept {
	return m_layers.empty();
}

bool nv::editor::NodeEditor::isBusy() const noexcept {
	return m_polygonBuilder.building() || m_creatingObjectGroup;
}

FileID nv::editor::NodeEditor::getID() const noexcept {
	return m_id;
}