#pragma once

#include <boost/unordered/unordered_flat_map.hpp>
#include <nlohmann/json.hpp>

#include "../reflection/TypeInfo.h"
#include "../serialization/PolygonSerialization.h"
#include "../serialization/SpritesheetSerialization.h"
#include "../serialization/TextureSerialization.h"
#include "../../BufferedNode.h"
#include "../../Instance.h"
#include "KeyConstants.h"

namespace nlohmann {
	using namespace nv::detail::json_constants;

	template<>
	struct nlohmann::adl_serializer<nv::BufferedNode> {
		template<typename T>
		static constexpr const std::string& typeOffsetKey() {
			static const auto typeStr = nv::detail::getTypeName<T>();
			static const auto combinedStr = std::string{ typeStr.data(), typeStr.size() } + "_Offset";
			return combinedStr;
		}
		template<typename T>
		static constexpr const std::string& typeSizeKey() {
			static const auto typeStr = nv::detail::getTypeName<T>();
			static const auto combinedStr = std::string{ typeStr.data(), typeStr.size() } + "_Size";
			return combinedStr;
		}
		template<typename T>
		static constexpr const std::string& typeCountKey() {
			static const auto typeStr = nv::detail::getTypeName<T>();
			static const auto combinedStr = std::string{ typeStr.data(), typeStr.size() } + "_Count";
			return combinedStr;
		}

		using RegionMap = nv::BufferedNode::TypeMap<nv::detail::MemoryRegion>;
		template<typename T>
		using ObjectMapEntry = nv::BufferedNode::ObjectMapEntry<T>;

		static nv::BufferedNode::ObjectLookups makeObjectMap(RegionMap& regionMap) {
			return nv::BufferedNode::ObjectLookups{
				regionMap.get<ObjectMapEntry<nv::Texture*>>().interpretAsSpan<ObjectMapEntry<nv::Texture*>>(),
				regionMap.get<ObjectMapEntry<nv::BufferedPolygon*>>().interpretAsSpan<ObjectMapEntry<nv::BufferedPolygon*>>(),
				regionMap.get<ObjectMapEntry<nv::BufferedNode*>>().interpretAsSpan<ObjectMapEntry<nv::BufferedNode*>>(),
				regionMap.get<ObjectMapEntry<nv::Spritesheet*>>().interpretAsSpan<ObjectMapEntry<nv::Spritesheet*>>()
			};
		}

		static RegionMap makeRegionMap(std::byte* arena, const json& root) {
			RegionMap ret;
			ret.forEach([&]<typename Object>(nv::detail::MemoryRegion & region) {
				//std::println("Offset Key: {}", typeOffsetKey<Object>());
				auto offset = root[typeOffsetKey<Object>()].get<size_t>();
				auto regionLen = root[typeSizeKey<Object>()].get<size_t>();
				region = { arena + offset, regionLen };
			});
			return ret;
		}

		static nv::BufferedNode::String makeBufferedString(const std::string& src, nv::detail::MemoryRegion& charRegion) {
			auto buff = charRegion.allocate<char>(src.size());
			strncpy(buff, src.data(), src.size());
			return { buff, src.size() };
		}

		template<typename T>
		static void createLookup(const json& metadataJson, nv::detail::MemoryRegion& charRegion, 
			nv::BufferedNode::Map<T*>& map, T* objectPtr) noexcept 
		{
			auto name = metadataJson[NAME_KEY].get<std::string>();
			if (!name.empty()) {
				map.insert(makeBufferedString(name, charRegion), objectPtr);
			}
		};

		class ObjectGroupCreator {
		private:
			using SpanIndices  = nv::detail::TypeMap<
				size_t,
				nv::Texture*, 
				nv::BufferedNode*, 
				nv::BufferedPolygon*,
				nv::Spritesheet*
			>;
			using SpanIndexMap = boost::unordered_flat_map<std::string, SpanIndices>;
			SpanIndexMap m_spanIndexMap;

			using ObjectGroupMap = nv::BufferedNode::ObjectGroupMap;

			ObjectGroupMap m_objectGroupMap;
		public:
			ObjectGroupCreator(RegionMap& regionMap, const json& root) 
				: m_objectGroupMap{ 
					regionMap.get<nv::BufferedNode::ObjectGroupMap::Entry>().interpretAsSpan<ObjectGroupMap::Entry>()
				}
			{
				auto& objectGroupNode = root[OBJECT_GROUP_KEY];
				for (const auto& objectGroupJson : objectGroupNode) {
					nv::BufferedNode::ObjectGroup objectGroup;

					//allocate memory for each object group
					nv::detail::forEachDataMember([&]<typename T>(std::span<T*>& span) {
						auto objectPtrCount = objectGroupJson[typeCountKey<T*>()].get<size_t>();
						if (objectPtrCount == 0) {
							return nv::detail::STAY_IN_LOOP;
						}
						auto objectPtrSpanPtr = regionMap.get<T*>().allocate<T*>(objectPtrCount);
						span = { objectPtrSpanPtr, objectPtrCount };
						return nv::detail::STAY_IN_LOOP;
					}, objectGroup);

					auto objectGroupName = makeBufferedString(objectGroupJson[NAME_KEY].get<std::string>(), regionMap.get<char>());
					m_objectGroupMap.insert(std::move(objectGroupName), std::move(objectGroup));
				}

				//initialize span index map
				for (const auto& [mapName, key, isTombstone] : m_objectGroupMap) {
					m_spanIndexMap.emplace(
						std::piecewise_construct, 
						std::tuple{ mapName.buff, mapName.len }, std::tuple{ 0 }
					);
				}
			}

			template<typename T>
			void add(const std::string& objectGroupName, T* objectPtr) {
				auto& objectGroupMap     = m_objectGroupMap.at(objectGroupName);
				auto& objectSpan         = std::get<std::span<T*>>(objectGroupMap);
				auto& objectPtrIdx       = m_spanIndexMap.at(objectGroupName).get<T*>();
				objectSpan[objectPtrIdx] = objectPtr;
				objectPtrIdx++;
			}

			ObjectGroupMap get() {
				return m_objectGroupMap;
			}
		};

		template<typename T>
		static void addToObjectGroups(const json& metadataJson, ObjectGroupCreator& objectGroupCreator, T* objectPtr) 
		{
			auto objectGroupNames = metadataJson[OBJECT_GROUP_KEY].get<std::vector<std::string>>();
			for (const auto& objectGroupName : objectGroupNames) {
				objectGroupCreator.add(objectGroupName, objectPtr);
			}
		}

		static nv::BufferedNode* loadChildNode(const json& nodeJson, RegionMap& regionMap)
		{
			auto& registry = nv::getGlobalInstance()->registry;
			auto childSrc = registry.loadBufferedNode(nodeJson[PATH_KEY]);
			auto childSrcArena = std::get<nv::detail::AlignedBuffer<>>(childSrc.m_arena).data;

			assert(childSrc.m_byteC > 0);

			auto& childNodeRegion = regionMap.get<nv::BufferedNode>();
			auto& childNodeArenaRegion = regionMap.get<std::byte>();

			auto childDest = childNodeRegion.emplace<nv::BufferedNode>();
			childDest->m_byteC = childSrc.m_byteC;
			childDest->m_arena = childNodeArenaRegion.allocate<std::byte>(childSrc.m_byteC);
			auto childDestArena = std::get<std::byte*>(childDest->m_arena);

			nv::BufferedNode::deepCopyChild(childSrc, childSrcArena, *childDest, childDestArena);

			//set metadata
			childDest->resetWorld();
			childDest->setOpacity(nodeJson[OPACITY_KEY].get<uint8_t>());
			childDest->screenScale(nodeJson[SCREEN_SCALE_KEY].get<float>());
			childDest->setScreenPos(nodeJson[SCREEN_POS_KEY].get<nv::Point>());
			childDest->setWorldPos(nodeJson[WORLD_POS_KEY].get<nv::Point>());

			return childDest;
		}

		static std::span<nv::BufferedNode*> parseNodeGroup(
			const json& nodeGroupJson, RegionMap& regionMap, nv::BufferedNode::Map<nv::BufferedNode*>& map,
			ObjectGroupCreator& objectGroupCreator)
		{
			//return default null span if there are no nodes in the group
			if (nodeGroupJson.size() == 0) {
				return std::span<nv::BufferedNode*>{};
			}

			auto& nodePtrRegion = regionMap.get<nv::BufferedNode*>();
			auto nodePtrArray = nodePtrRegion.allocate<nv::BufferedNode*>(nodeGroupJson.size());
			std::span nodePtrSpan{ nodePtrArray, nodeGroupJson.size() };

			for (const auto& [idx, nodeJson] : std::views::enumerate(nodeGroupJson)) {
				auto& childNodePtrRegion = regionMap.get<nv::BufferedNode*>();
				auto& metadataJson = nodeJson[METADATA_KEY];

				auto objectPtr = loadChildNode(metadataJson, regionMap);
				nodePtrSpan[idx] = objectPtr;
				createLookup(metadataJson, regionMap.get<char>(), map, objectPtr);
				addToObjectGroups(metadataJson, objectGroupCreator, objectPtr);
			}
			
			return nodePtrSpan;
		}

		template<typename Object>
		static std::span<Object> parseObjectGroup(const json& objectGroupJson,
			nv::detail::MemoryRegion& charRegion, nv::detail::MemoryRegion& objectRegion,
			nv::BufferedNode::Map<Object*>& map, ObjectGroupCreator& objectGroupCreator)
		{
			//if there are no objects in the group, return default span
			if (objectGroupJson.size() == 0) {
				return std::span<Object>{};
			}
			auto objectGroupPtr = objectRegion.allocate<Object>(objectGroupJson.size());
			std::span<Object> objectSpan{ objectGroupPtr, objectGroupJson.size() };
			
			for (const auto& [idx, json] : std::views::enumerate(objectGroupJson)) {
				auto& objectJson = json[OBJECT_KEY];
				auto& metadataJson = json[METADATA_KEY];
				auto objectSpanPtr = objectSpan.data() + idx;
				objectSpanPtr = new (objectSpanPtr) Object{ objectJson.get<Object>() };
				createLookup(metadataJson, charRegion, map, &objectSpan[idx]);
				addToObjectGroups(metadataJson, objectGroupCreator, &objectSpan[idx]);
			}

			return objectSpan;
		}

		static nv::BufferedNode from_json(const json& root) {
			nv::BufferedNode ret;
			auto byteC = root[BYTES_KEY].get<size_t>();
			ret.m_arena = byteC;
			ret.m_byteC = byteC;

			auto regionMap      = makeRegionMap(std::get<nv::detail::AlignedBuffer<std::byte>>(ret.m_arena).data, root);
			ret.m_objectLookups = makeObjectMap(regionMap);

			//make layer map
			using LayerMapEntry = nv::BufferedNode::LayerMap::Entry;
			ret.m_layerMap.arr = regionMap.get<LayerMapEntry>().interpretAsSpan<LayerMapEntry>();

			ObjectGroupCreator objectGroupCreator{ regionMap, root };

			//allocate layers
			auto& layersJson = root[LAYERS_KEY];
			auto& layerRegion = regionMap.get<nv::BufferedNode::Layer>();
			ret.m_objectLayers = { layerRegion.allocate<nv::BufferedNode::Layer>(
				layersJson.size()
			), layersJson.size() };

			//make polygon serializer allocate memory from our specified point region
			for (auto&& [objectLayer, jsonLayer] : std::views::zip(ret.m_objectLayers, layersJson)) {
				//reset point region in case we loaded a child node 
				auto pointRegion = &regionMap.get<nv::Point>();
				adl_serializer<nv::detail::Polygon<std::span<nv::Point>>>::currentPointRegion = pointRegion;

				//create layer lookup
				auto layerName = jsonLayer[NAME_KEY].get<std::string>();
				if (!layerName.empty()) {
					auto bufferedLayerName = makeBufferedString(layerName, regionMap.get<char>());
					ret.m_layerMap.insert(bufferedLayerName, &objectLayer);
				}
				
				nv::detail::forEachDataMember([&]<typename Object>(std::span<Object>& objectSpan) {
					objectSpan = std::span<Object>{}; //default initialize object span

					//remove pointer for nv::BufferedNode*, whose key is nv::BufferedNode
					auto typeName = nv::detail::getTypeName<std::remove_pointer_t<Object>>();
					auto objectGroupJsonIt = jsonLayer.find(typeName);
					if (objectGroupJsonIt == jsonLayer.end()) {
						return nv::detail::STAY_IN_LOOP;
					}

					auto& objectGroupJson = *objectGroupJsonIt;

					using ObjectMap = nv::BufferedNode::Map<std::remove_pointer_t<Object>*>;

					auto& objectRegion     = regionMap.get<Object>();
					auto objectLayerRegion = objectRegion.makeSubregion(0, objectGroupJson.size() * sizeof(Object));
					auto& objectMap        = std::get<ObjectMap>(ret.m_objectLookups);
					
					if constexpr (std::same_as<Object, nv::BufferedNode*> || std::same_as<Object, nv::BufferedNode>) {
						objectSpan = parseNodeGroup(
							objectGroupJson, regionMap, objectMap, objectGroupCreator
						);
					} else {
						objectSpan = parseObjectGroup(
							objectGroupJson, regionMap.get<char>(), objectRegion, objectMap, objectGroupCreator
						);
					}

					return nv::detail::STAY_IN_LOOP;
				}, objectLayer);
			}

			ret.m_objectGroupMap = objectGroupCreator.get();

			return ret;
		}
	};
}