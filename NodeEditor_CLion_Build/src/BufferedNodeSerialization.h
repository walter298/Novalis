#pragma once

#include "Layer.h"

namespace nv {
	namespace editor {
		static BufferedNode::TypeMap<size_t> calculateObjectRegionOffsets(const BufferedNode::TypeMap<size_t>& objectRegionLengths) {
			size_t currOffset = 0;
			BufferedNode::TypeMap<size_t> offsets{ 0 };
			objectRegionLengths.forEach([&]<typename Object>(size_t len) {
				currOffset = (currOffset + alignof(Object) - 1) & ~(alignof(Object) - 1);
				offsets.get<Object>() = currOffset;
				currOffset += len;
			});
			return offsets;
		}

		template<bool IsBase = true, typename T>
		static constexpr void calculateSizeBytes(const T& t, BufferedNode::TypeMap<size_t>& objectRegionLengths) {
			if constexpr (concepts::Primitive<T>) {
				objectRegionLengths.get<T>() += sizeof(T);
			}
			else if constexpr (std::ranges::viewable_range<T>) {
				using ValueType = typename T::value_type;
				if constexpr (concepts::Primitive<ValueType>) {
					objectRegionLengths.get<ValueType>() += (sizeof(ValueType) * std::ranges::size(t));
				}
				else {
					for (const auto& elem : t) {
						calculateSizeBytes(elem, objectRegionLengths);
					}
				}
			} else {
				if constexpr (IsBase) {
					objectRegionLengths.get<T>() += sizeof(T);
				}
				nv::detail::forEachDataMember([&]<typename Field>(const Field & field) {
					if constexpr (!concepts::Primitive<Field>) {
						calculateSizeBytes<false>(field, objectRegionLengths);
					}
					return nv::detail::STAY_IN_LOOP;
				}, t);
			}
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
			nv::detail::forEachDataMember([&]<typename Object>(const EditedObjectHive<Object>&hive) {
				using BufferedObject = std::remove_cvref_t<decltype(makeBufferedObject(std::declval<Object>()))>;

				auto typeName = nv::detail::getTypeName<BufferedObject>();

				auto& objGroup = currJsonLayer[typeName] = json::array();
				for (const auto& obj : hive) {
					decltype(auto) bufferedObject = makeBufferedObject(obj.obj);
					if constexpr (std::same_as<std::remove_cvref_t<decltype(obj.obj)>, DynamicPolygon>) {
						std::println("Hit polygon!");
					}
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

				return nv::detail::STAY_IN_LOOP;
			}, objects);
		}

		std::string createNodeJson(const std::vector<Layer>& layers, std::string_view nodeName) {
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
				root[BufferedNodeParser::typeOffsetKey<Object>()] = offset;
			});

			root[BYTES_KEY] = offsets.getLast() + objectRegionLengths.getLast();
			return root.dump(2);
		}
	}
}