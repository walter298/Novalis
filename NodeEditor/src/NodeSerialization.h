#pragma once

#include <novalis/detail/serialization/KeyConstants.h>

#include "EditedObjectGroup.h"
#include "Layer.h"
#include "ObjectByteSizeCalculator.h"

namespace nv {
	namespace editor {
		template<typename Object>
		void roundUpToNearestAlignment(size_t& n) {
			n = (n + alignof(Object) - 1) & ~(alignof(Object) - 1);
		}

		static BufferedNode::TypeMap<size_t> calculateObjectRegionOffsets(const BufferedNode::TypeMap<size_t>& objectRegionLengths) {
			auto currOffset = alignof(std::max_align_t);
			BufferedNode::TypeMap<size_t> offsets{ 0 };
			objectRegionLengths.forEach([&]<typename Object>(size_t len) {
				// roundUpToNearestAlignment<Object>(currOffset);
				// offsets.get<Object>() = currOffset;
				// currOffset += len;
			});
			return offsets;
		}

		template<typename T>
		static decltype(auto) makeBufferedObject(const T& t) {
			if constexpr (std::same_as<T, DynamicPolygon>) {
				return nv::detail::PolygonConverter::makeBufferedPolygon(t);
			} else {
				return (t);
			}
		}

		static void writeObjectData(json& currJsonLayer, const Layer::Objects& objects, BufferedNode::TypeMap<size_t>& objectRegionLengths) {
			nv::detail::forEachDataMember([&]<typename Object>(const EditedObjectHive<Object>& hive) {
				using BufferedObject = std::remove_cvref_t<decltype(makeBufferedObject(std::declval<Object>()))>;

				auto typeName = nv::detail::getTypeName<BufferedObject>();

				auto& objGroup = currJsonLayer[typeName] = json::array();
				for (const auto& obj : hive) {
					decltype(auto) bufferedObject = makeBufferedObject(obj.obj);
					if constexpr (std::same_as<nv::BufferedNode, BufferedObject>) {
						//broken!
						/*objectRegionLengths.get<std::byte*>() += obj.obj.getSizeBytes();
						objectRegionLengths.get<BufferedNode>() += sizeof(BufferedNode);
						objectRegionLengths.get<BufferedNode*>() += sizeof(BufferedNode*);*/
					} else {
						calculateSizeBytes(bufferedObject, objectRegionLengths);
					}

					objGroup.emplace_back() = obj;

					if (!obj.name.empty()) {
						using ObjectMapEntry = BufferedNode::ObjectMapEntry<std::remove_pointer_t<BufferedObject>*>;
						objectRegionLengths.get<char>() += obj.name.size();
						objectRegionLengths.get<ObjectMapEntry>() += sizeof(ObjectMapEntry);
					}
				}

				return nv::detail::STAY_IN_LOOP;
			}, objects);
		}

		std::string createNodeJson(const std::vector<Layer>& layers, std::string_view nodeName) {
			using namespace nv::detail::json_constants;

			json root;
			root[NAME_KEY] = nodeName;
			auto& layersRoot = root[LAYERS_KEY] = json::array();

			BufferedNode::TypeMap<size_t> objectRegionLengths{ 0 };
			objectRegionLengths.get<BufferedNode::Layer>() = layers.size() * sizeof(BufferedNode::Layer);

			for (const auto& [layerName, objects] : layers) {
				auto& currJsonLayer = layersRoot.emplace_back();

				currJsonLayer[NAME_KEY] = layerName;
				objectRegionLengths.get<char>() += layerName.size();

				writeObjectData(currJsonLayer, objects, objectRegionLengths);
			}

			using BufferedNodeParser = nlohmann::adl_serializer<BufferedNode>;
			objectRegionLengths.forEach([&]<typename Object>(size_t size) {
				root[BufferedNodeParser::typeSizeKey<Object>()] = size;
			});

			auto offsets = calculateObjectRegionOffsets(objectRegionLengths);
			offsets.forEach([&]<typename Object>(size_t offset) {
				//root[BufferedNodeParser::typeOffsetKey<Object>()] = offset;
			});

			root[BYTES_KEY] = offsets.getLast() + objectRegionLengths.getLast();
			return root.dump(2);
		}
	}
}