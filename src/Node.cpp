//#include "Node.h"
//
//#include <print>
//#include <fstream>
//
//#include "data_util/Reflection.h"
//
////namespace nlohmann {
////	template<>
////	struct adl_serializer<nv::BufferedNode> {
////		static constexpr const char* STATIC_NODE_SIZE = "STATIC_NODE_SIZE";
////		static constexpr const char* LAYERS_KEY = "Layers";
////		static constexpr const char* LAYER_NUMBER_KEY = "Number";
////		static constexpr const char* LAYER_OBJECTS_KEY = "Objects";
////		static constexpr const char* BUFFERED_NODE_KEY = "Buffered_Node";
////		static constexpr const char* DYNAMIC_NODE_KEY = "Dynamic_Node";
////		static constexpr const char* OBJECT_NAME_KEY = "Name";
////
////		/*SDL_Renderer* globalRenderer = nullptr;
////		nv::Arena* currentArena = nullptr;
////		nv::Instance* globalInstance = nullptr;*/
////
////		static void to_json(json& j, const nv::BufferedNode& node) {
////
////		}
////
////		static nv::BufferedNode from_json(const json& j) {
////			auto globalInstance = nv::getGlobalInstance();
////			auto nodeName = j[OBJECT_NAME_KEY].get<std::string>();
////			auto nodeIt = globalInstance->bufferedNodeMap.find(nodeName);
////			if (nodeIt != globalInstance->bufferedNodeMap.end()) {
////				return nodeIt->second;
////			}
////
////			nv::BufferedNode node{ j[STATIC_NODE_SIZE].get<size_t>() };
////			
////			auto loadObjectLayers = [&](const json& objsJson, int layer) {
////				node.m_objectLayers.forEachHive([&](int layer, auto& objectSpan) {
////					using Type = typename std::remove_cvref_t<decltype(objectSpan)>::value_type;
////					auto jsonStoredObjects = objsJson[typeid(Type).name()].get<std::vector<Type>>();
////					
////					auto objArray = node.m_arena.allocate(jsonStoredObjects.size_bytes(), alignof(Type));
////					assert(objArray != nullptr);
////
////					objectSpan = { objArray, jsonStoredObjects.size() };
////
////					std::ranges::move(jsonStoredObjects, objectSpan.begin());
////
////					return nv::STAY_IN_LOOP;
////				});
////			};
////
////			auto layers = j[LAYERS_KEY].array();
////			for (const auto& layer : layers) {
////				loadObjectLayers(layer, layer[LAYER_NUMBER_KEY].get<int>());
////			}
////
////			globalInstance->bufferedNodeMap.emplace(nodeName, node);
////			return node;
////		}
////	};
////}
//
////void nv::Node::render(SDL_Renderer* renderer) const noexcept {
////	m_objectLayers.forEach([&, this](int layer, const auto& obj) {
////		using Type = std::remove_cvref_t<decltype(obj)>;
////		if constexpr (IsClassTemplate<boost::local_shared_ptr, Type>::value || std::is_pointer_v<Type>) {
////			obj->render(renderer);
////		} else {
////			unrefwrap(obj).render(renderer);
////		}
////		return STAY_IN_LOOP;
////	});
////}
//
////nv::BufferedNode nv::BufferedNode::load(const char* path) {
////	std::ifstream file{ path };
////	auto root = json::parse(file);
////	return root.get<BufferedNode>();
////}
//
