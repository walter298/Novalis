#pragma once

#include <nlohmann/json.hpp>

#include "../reflection/TypeInfo.h"
#include "../serialization/PolygonSerialization.h"
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

		using RegionMap = nv::BufferedNode::TypeMap<nv::detail::MemoryRegion>;
		template<typename T>
		using ObjectMapEntry = nv::BufferedNode::ObjectMapEntry<T>;

		static nv::BufferedNode::ObjectLookups makeObjectMap(RegionMap& regionMap) {
			return nv::BufferedNode::ObjectLookups{
				regionMap.get<ObjectMapEntry<nv::Texture>>(),
				regionMap.get<ObjectMapEntry<nv::BufferedPolygon>>(),
				regionMap.get<ObjectMapEntry<nv::BufferedNode>>()
			};
		}

		static RegionMap makeRegionMap(std::byte* arena, const json& root) {
			RegionMap ret;
			ret.forEach([&]<typename Object>(nv::detail::MemoryRegion & region) {
				auto offset = root[typeOffsetKey<Object>()].get<size_t>();
				auto regionLen = root[typeSizeKey<Object>()].get<size_t>();
				region = { arena + offset, regionLen };
			});
			return ret;
		}

		template<typename T>
		static void createLookup(const json& metadataJson, nv::detail::MemoryRegion& charRegion, 
			nv::BufferedNode::Map<T>& map, T* objectPtr) noexcept 
		{
			auto name = metadataJson[NAME_KEY].get<std::string>();
			if (!name.empty()) {
				auto buff = charRegion.allocate<char>(name.size());
				strncpy(buff, name.data(), name.size());
				map.insert({ buff, name.size() }, objectPtr);
			}
		};

		static nv::BufferedNode* loadChildNode(const json& nodeJson, 
			nv::detail::MemoryRegion& childNodeRegion, nv::detail::MemoryRegion& childNodeArenaRegion)
		{
			auto& registry = nv::getGlobalInstance()->registry;
			auto child = registry.loadBufferedNode(nodeJson[PATH_KEY]);

			auto childCopy = childNodeRegion.allocate<nv::BufferedNode>(1);
			childCopy->m_byteC = child.m_byteC;
			childCopy->m_arena = childNodeArenaRegion.allocate<std::byte>(child.m_byteC);
			nv::BufferedNode::deepCopyChild(child, std::get<std::byte*>(child.m_arena),
				*childCopy, std::get<std::byte*>(childCopy->m_arena));
			childCopy->setOpacity(nodeJson[OPACITY_KEY]);

			return childCopy;
		}

		static std::span<nv::BufferedNode*> parseNodeGroup(
			const json& nodeGroupJson, nv::detail::MemoryRegion& charRegion,
			nv::detail::MemoryRegion childNodePtrRegion, nv::detail::MemoryRegion& childNodeRegion, 
			nv::detail::MemoryRegion& childNodeArenaRegion, nv::BufferedNode::Map<nv::BufferedNode>& map)
		{
			for (const auto& nodeJson : nodeGroupJson) {
				auto objectPtr = childNodePtrRegion.emplace<nv::BufferedNode*>(
					loadChildNode(nodeJson[OBJECT_KEY], childNodeRegion, childNodeArenaRegion)
				);
				createLookup(nodeJson[METADATA_KEY], charRegion, map, *objectPtr);
			}
			return childNodePtrRegion.interpretAsSpan<nv::BufferedNode*>();
		}

		template<typename Object>
		static std::span<Object> parseObjectGroup(const json& objectGroupJson,
			nv::detail::MemoryRegion& charRegion, nv::detail::MemoryRegion& objectRegion,
			nv::BufferedNode::Map<Object>& map)
		{
			static size_t pointBytesAllocated = 0;
			static size_t polygonC = 0;

			static size_t unaccountedForPolygons = 0;

			size_t i = 0;

			std::println("Actual # of polygons: {}", objectGroupJson.size());

			for (const auto& json : objectGroupJson) {
				auto& objectJson = json[OBJECT_KEY];
				auto& metadataJson = json[METADATA_KEY];

				auto pointRegion = adl_serializer<nv::BufferedPolygon>::currentPointRegion;
				if (pointRegion->getSpace() == 0 && std::same_as<Object, nv::BufferedPolygon>) {
					unaccountedForPolygons += (objectGroupJson.size() - i);
					return objectRegion.interpretAsSpan<Object>();
				}

				auto object = objectJson.get<Object>();

				auto objectPtr = objectRegion.emplace<Object>(std::move(object));
				createLookup(metadataJson, charRegion, map, objectPtr);

				i++;
			}
			return objectRegion.interpretAsSpan<Object>();
		}

		static nv::BufferedNode from_json(const json& root) {
			nv::BufferedNode ret;
			auto byteC = root[BYTES_KEY].get<size_t>();
			ret.m_arena = byteC;
			ret.m_byteC = byteC;

			auto regionMap = makeRegionMap(std::get<nv::detail::AlignedBuffer<std::byte>>(ret.m_arena).data, root);
			ret.m_objectLookups = makeObjectMap(regionMap);
			
			auto& layerRegion = regionMap.get<nv::BufferedNode::Layer>();
			ret.m_objectLayers = layerRegion.interpretAsSpan<nv::BufferedNode::Layer>();

			auto& jsonLayers = root[LAYERS_KEY];
			
			//make polygon serializer allocate memory from our specified point region
			auto pointRegion = &regionMap.get<nv::Point>();
			adl_serializer<nv::BufferedPolygon>::currentPointRegion = pointRegion;

			std::println("Point space before: {}", pointRegion->getSpace());

			std::println("Json Layers Size: {}", jsonLayers.size());

			for (auto&& [layerIdx, jsonLayer] : std::views::enumerate(jsonLayers)) {
				nv::BufferedNode::Layer nodeLayer;

				nv::detail::forEachDataMember([&]<typename Object>(std::span<Object>& objectSpan) {
					auto typeName = nv::detail::getTypeName<Object>();
					auto objectGroupJson = jsonLayer.find(typeName);
					if (objectGroupJson == jsonLayer.end()) {
						return nv::detail::STAY_IN_LOOP;
					}

					using ObjectMap = nv::BufferedNode::Map<std::remove_pointer_t<Object>>;

					if constexpr (std::same_as<Object, nv::BufferedPolygon>) {
						std::println("{} # of polygons", objectGroupJson->size());
					}
					auto& objectRegion     = regionMap.get<Object>();
					auto objectLayerRegion = objectRegion.makeSubregion(0, objectGroupJson->size() * sizeof(Object));
					auto& objectMap        = std::get<ObjectMap>(ret.m_objectLookups);
					auto& charRegion       = regionMap.get<char>();

					if constexpr (!std::same_as<Object, nv::BufferedNode*>) {
						objectSpan = parseObjectGroup<Object>(
							*objectGroupJson, charRegion, objectRegion, objectMap
						);
					}

					//for (const auto& objectJson : *objectGroupJson) {
					//	if constexpr (std::same_as<Object, nv::BufferedNode*>) {
					//		/*auto& childNodeRegion = regionMap.get<nv::BufferedNode*>();
					//		auto& childNodeArenaRegion = regionMap.get<std::byte*>();
					//		objectSpan = parseNodeGroup(
					//			objGroupJson, charRegion, objectRegion, childNodeRegion,
					//			childNodeArenaRegion, objectMap
					//		);*/
					//	} else {
					//		/*objectSpan = parseObjectGroup<Object>(
					//			objGroupJson, charRegion, objectRegion, objectMap
					//		);*/
					//		if constexpr (std::same_as<Object, nv::BufferedPolygon>) {
					//			objectSpan = parseObjectGroup<Object>(
					//				*objectGroupJson, charRegion, objectRegion, objectMap
					//			);
					//		}
					//	}
					//}

					return nv::detail::STAY_IN_LOOP;
				}, nodeLayer);
				ret.m_objectLayers.storage[layerIdx] = std::move(nodeLayer);
			}

			std::println("Point space after: {}", pointRegion->getSpace());
			
			return ret;
		}
	};
}