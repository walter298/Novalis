#pragma once

#include <novalis/detail/serialization/KeyConstants.h>

#include "ObjectGroupManager.h"
#include "Layer.h"
#include "ObjectByteSizeCalculator.h"

namespace nv {
	namespace editor {
		template<typename Object>
		static void roundUpToNearestAlignment(size_t& n) {
			n = (n + alignof(Object) - 1) & ~(alignof(Object) - 1);
		}

		static BufferedNode::TypeMap<size_t> calculateObjectRegionOffsets(const BufferedNode::TypeMap<size_t>& objectRegionLengths) {
			auto currOffset = alignof(std::max_align_t);
			BufferedNode::TypeMap<size_t> offsets{ 0 };
			objectRegionLengths.forEach([&]<typename Object>(size_t len) {
				roundUpToNearestAlignment<Object>(currOffset);
				offsets.get<Object>() = currOffset;
				currOffset += len;
			});
			return offsets;
		}

		template<typename Object>
		static json makeObjectJson(const EditedObjectData<Object>& obj, const ObjectGroupManager& objectGroups) {
			json objectRootJson;

			auto& metadataJson = objectRootJson[METADATA_KEY];
			auto& objectJson = objectRootJson[OBJECT_KEY];

			metadataJson[NAME_KEY] = obj.getName();

			//write object group data
			auto& objectGroupsJson = metadataJson[OBJECT_GROUP_KEY] = json::array();
			for (const auto& id : obj.groupIDs) {
				const auto& objectGroup = objectGroups.getGroup(id);
				objectGroupsJson.push_back(objectGroup.name);
			}

			if constexpr (std::same_as<Object, BufferedNode>) {
				metadataJson[PATH_KEY] = obj.filePath;
				metadataJson[OPACITY_KEY] = obj.obj.getOpacity();
				metadataJson[SCREEN_SCALE_KEY] = obj.obj.getScreenScale();
				metadataJson[SCREEN_POS_KEY] = obj.obj.getScreenPos();
				metadataJson[WORLD_POS_KEY] = obj.obj.getWorldPos();
			} else if constexpr (std::same_as<Object, Texture>) {
				objectJson[RENDER_DATA_KEY] = obj.obj.texData;
				objectJson[IMAGE_PATH_KEY] = obj.texPath;
			} else {
				objectJson = obj.obj;
			}

			return objectRootJson;
		}

		template<typename Object>
		using BufferedObject = std::conditional_t<
			std::same_as<Object, DynamicPolygon>, 
			BufferedPolygon,
			Object
		>;

		static void writeLayerData(json& currJsonLayer, const Layer::Objects& objects, 
			BufferedNode::TypeMap<size_t>& objectRegionLengths, ObjectGroupManager& objectGroups) 
		{
			auto handleObject = [&]<typename Object>(json& objectGroupJson, const EditedObjectData<Object>& object) {
				objectGroupJson.push_back(makeObjectJson(object, objectGroups));

				if constexpr (std::same_as<Object, nv::DynamicPolygon>) {
					auto bufferedPolygon = nv::detail::PolygonConverter::makeBufferedPolygon(object.obj);
					calculateSizeBytes(bufferedPolygon, objectRegionLengths);
				} else if constexpr (!std::same_as<Object, nv::BufferedNode>) {
					calculateSizeBytes(object.obj, objectRegionLengths);
				}

				if (!object.getName().empty()) {
					using ObjectMapEntry = BufferedNode::ObjectMapEntry<
						std::remove_pointer_t<BufferedObject<Object>>* //buffered nodes are stored as pointers, but we don't need to store double pointers to them
					>;
					objectRegionLengths.get<char>() += object.getName().size();
					objectRegionLengths.get<ObjectMapEntry>() += sizeof(ObjectMapEntry);
				}
			};


			nv::detail::forEachDataMember([&]<typename Object>(const EditedObjectHive<Object>& hive) {
				auto typeName = nv::detail::getTypeName<BufferedObject<Object>>();
				auto& objGroup = currJsonLayer[typeName] = json::array();

				for (const auto& object : hive) {
					handleObject(objGroup, object);
				}

				return nv::detail::STAY_IN_LOOP;
			}, objects);
		}

		static void writeAllLayersData(json& root, const std::vector<Layer>& layers, 
			ObjectGroupManager& objectGroups, BufferedNode::TypeMap<size_t>& objectRegionLengths)
		{
			objectRegionLengths.get<BufferedNode::Layer>() = layers.size() * sizeof(BufferedNode::Layer);

			auto& layersRoot = root[LAYERS_KEY] = json::array();

			for (const auto& [layerName, objects] : layers) {
				auto& currJsonLayer = layersRoot.emplace_back();

				currJsonLayer[NAME_KEY] = layerName;
				objectRegionLengths.get<char>() += layerName.size();

				writeLayerData(currJsonLayer, objects, objectRegionLengths, objectGroups);
			}
		}

		static void writeObjectSizeOffsetData(json& root, const BufferedNode::TypeMap<size_t>& objectRegionLengths) {
			using BufferedNodeParser = nlohmann::adl_serializer<BufferedNode>;
			objectRegionLengths.forEach([&]<typename Object>(size_t size) {
				root[BufferedNodeParser::typeSizeKey<BufferedObject<Object>>()] = size;
			});

			auto offsets = calculateObjectRegionOffsets(objectRegionLengths);
			offsets.forEach([&]<typename Object>(size_t offset) {
				root[BufferedNodeParser::typeOffsetKey<BufferedObject<Object>>()] = offset;
			});

			root[BYTES_KEY] = offsets.getLast() + objectRegionLengths.getLast();
		}

		static void writeObjectGroupData(json& root, ObjectGroupManager& objectGroups, 
			BufferedNode::TypeMap<size_t>& objectRegionLengths) 
		{
			auto& objectGroupNode = root[OBJECT_GROUP_KEY];
			for (const auto& [groupID, objectGroup] : objectGroups.getAllGroups()) {
				//add capacity for lookup entry and characters for the object group's name
				objectRegionLengths.get<BufferedNode::ObjectGroupMap::Entry>() += sizeof(BufferedNode::ObjectGroupMap::Entry);
				objectRegionLengths.get<char>() += objectGroup.name.size();

				//write json data
				json objectGroupJson;
				objectGroupJson[NAME_KEY] = objectGroup.name;
				nv::detail::forEachDataMember([&]<typename T>(const ObjectLookup<T>& objects) {
					using BufferedNodeParser = nlohmann::adl_serializer<nv::BufferedNode>;
					objectGroupJson[BufferedNodeParser::typeCountKey<BufferedObject<T>*>()] = objects.size();
					objectRegionLengths.get<BufferedObject<T>*>() += (objects.size() * sizeof(BufferedObject<T>*));
					return nv::detail::STAY_IN_LOOP;
				}, objectGroup.objects);
				objectGroupNode.push_back(std::move(objectGroupJson));
			}
		}

		inline std::string createNodeJson(const std::vector<Layer>& layers, std::string_view nodeName, ObjectGroupManager& objectGroups) {
			using namespace nv::detail::json_constants;

			json root;
			root[NAME_KEY] = nodeName;
			
			BufferedNode::TypeMap<size_t> objectRegionLengths{ 0 };

			writeObjectGroupData(root, objectGroups, objectRegionLengths);
			writeAllLayersData(root, layers, objectGroups, objectRegionLengths);
			writeObjectSizeOffsetData(root, objectRegionLengths);

			return root.dump(2);
		}
	}
}