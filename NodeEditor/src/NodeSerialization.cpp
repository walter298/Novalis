#include "NodeSerialization.h"

#include <novalis/detail/serialization/BufferedNodeSerialization.h>
#include <novalis/detail/serialization/PolygonSerialization.h>

#include "BufferedAlias.h"

using namespace nv;
using namespace editor;

template<typename Object>
static void roundUpToNearestAlignment(size_t& n) {
	constexpr size_t alignment = std::same_as<Object, std::byte> ? alignof(std::max_align_t) : alignof(Object);
	n = (n + alignment - 1) & ~(alignment - 1);
}

template<typename T>
struct IsStdArray : public std::false_type {};

template<typename T, size_t N>
struct IsStdArray<std::array<T, N>> : public std::true_type {};

template<typename T>
concept StaticArray = (std::is_array_v<T> && std::extent_v<T> > 0) || IsStdArray<T>::value;

template<bool IsBase = true, typename T>
static constexpr void calculateSizeBytes(const T& t, BufferedNode::TypeMap<size_t>& objectRegionLengths) {
	if constexpr (IsBase) {
		roundUpToNearestAlignment<T>(objectRegionLengths.get<T>());
	}

	if constexpr (concepts::Primitive<T>) {
		objectRegionLengths.get<T>() += sizeof(T);
	} else if constexpr (std::ranges::viewable_range<T>) {
		using ValueType = typename T::value_type;
		if constexpr (!StaticArray<T>) {
			for (const auto& elem : t) {
				calculateSizeBytes<true>(elem, objectRegionLengths);
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

static void writeNodeMetadata(json& metadataJson, const EditedObjectData<BufferedNode>& editedNode) {
	metadataJson[PATH_KEY] = editedNode.filePath;
	metadataJson[OPACITY_KEY] = editedNode.obj.getOpacity();
	metadataJson[SCREEN_SCALE_KEY] = editedNode.obj.getScreenScale();
	metadataJson[SCREEN_POS_KEY] = editedNode.obj.getScreenPos();
	metadataJson[WORLD_POS_KEY] = editedNode.obj.getWorldPos();
}

template<typename Object>
static void writeTextureDataJson(json& objectJson, const EditedObjectData<Object>& object) {
	objectJson[RENDER_DATA_KEY] = static_cast<nv::detail::TextureRenderData>(object.obj);
	objectJson[IMAGE_PATH_KEY] = object.texPath;
}

static void writeSpritesheetJson(json& objectJson, const EditedObjectData<Spritesheet>& editedSpritesheet) {
	using S = nlohmann::adl_serializer<Spritesheet>;
	objectJson[S::ROWS_KEY] = editedSpritesheet.obj.getRowCount();
	objectJson[S::COLS_KEY] = editedSpritesheet.obj.getColumnCount();
	writeTextureDataJson(objectJson, editedSpritesheet);
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
		writeNodeMetadata(metadataJson, obj);
	} else if constexpr (std::same_as<Object, Texture>) {
		writeTextureDataJson(objectJson, obj);
	} else if constexpr (std::same_as<Object, Spritesheet>) {
		writeSpritesheetJson(objectJson, obj);
	} else {
		nlohmann::adl_serializer<nv::DynamicPolygon>::to_json(objectJson, obj.obj); //todo: make function that does this
	}

	return objectRootJson;
}

static void writeChildNodeSizeData(const BufferedNode& node, BufferedNode::TypeMap<size_t>& objectRegionLengths) {
	objectRegionLengths.get<BufferedNode*>() += sizeof(BufferedNode*);
	objectRegionLengths.get<BufferedNode>() += sizeof(BufferedNode);
	objectRegionLengths.get<std::byte>() += node.getSizeBytes();
	roundUpToNearestAlignment<std::byte>(objectRegionLengths.get<std::byte>());
}

static void writeLayerData(json& currJsonLayer, const Layer::Objects& objects,
	BufferedNode::TypeMap<size_t>& objectRegionLengths, const ObjectGroupManager& objectGroups)
{
	auto handleObject = [&]<typename Object>(json & objectGroupJson, const EditedObjectData<Object>&object) {
		objectGroupJson.push_back(makeObjectJson(object, objectGroups));

		//calculate size of the object
		if constexpr (std::same_as<Object, nv::DynamicPolygon>) {
			auto bufferedPolygon = nv::detail::PolygonConverter::makeBufferedPolygon(object.obj);
			calculateSizeBytes(bufferedPolygon, objectRegionLengths);
		}
		else if constexpr (std::same_as<Object, nv::BufferedNode>) {
			writeChildNodeSizeData(object.obj, objectRegionLengths);
		} else {
			calculateSizeBytes(object.obj, objectRegionLengths);
		}

		assert(!object.getName().empty());

		using ObjectMapEntry = BufferedNode::ObjectMapEntry<
			std::remove_pointer_t<BufferedObject<Object>>* //buffered nodes are stored as pointers, but we don't need to store double pointers to them
		>;
		objectRegionLengths.get<char>() += object.getName().size();
		objectRegionLengths.get<ObjectMapEntry>() += sizeof(ObjectMapEntry);
	};

	nv::detail::forEachDataMember([&]<typename Object>(const EditedObjectHive<Object>&hive) {
		auto typeName = nv::detail::getTypeName<BufferedObject<Object>>();
		auto& objGroup = currJsonLayer[typeName] = json::array();

		for (const auto& object : hive) {
			handleObject(objGroup, object);
		}

		return nv::detail::STAY_IN_LOOP;
	}, objects);
}

static void writeAllLayersData(json& root, const std::vector<Layer>& layers,
	const ObjectGroupManager& objectGroups, BufferedNode::TypeMap<size_t>& objectRegionLengths)
{
	objectRegionLengths.get<BufferedNode::Layer>() = layers.size() * sizeof(BufferedNode::Layer);

	auto& layersRoot = root[LAYERS_KEY] = json::array();

	for (const auto& [layerName, objects] : layers) {
		assert(!layerName.empty());

		auto& currJsonLayer = layersRoot.emplace_back();
		
		currJsonLayer[NAME_KEY] = layerName;
		objectRegionLengths.get<char>() += layerName.size();
		objectRegionLengths.get<BufferedNode::LayerMap::Entry>() += sizeof(BufferedNode::LayerMap::Entry);
		writeLayerData(currJsonLayer, objects, objectRegionLengths, objectGroups);
	}
}

//returns the total byte count of the node
static size_t writeObjectSizeOffsetData(json& root, const BufferedNode::TypeMap<size_t>& objectRegionLengths) {
	using BufferedNodeParser = nlohmann::adl_serializer<BufferedNode>;
	objectRegionLengths.forEach([&]<typename Object>(size_t size) {
		root[BufferedNodeParser::typeSizeKey<BufferedObject<Object>>()] = size;
	});

	auto offsets = calculateObjectRegionOffsets(objectRegionLengths);
	offsets.forEach([&]<typename Object>(size_t offset) {
		root[BufferedNodeParser::typeOffsetKey<BufferedObject<Object>>()] = offset;
	});

	size_t byteCount = offsets.getLast() + objectRegionLengths.getLast();
	root[BYTES_KEY] = byteCount;

	return byteCount;
}

static void writeObjectGroupData(json& root, const ObjectGroupManager& objectGroups,
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
		nv::detail::forEachDataMember([&]<typename T>(const ObjectLookup<T>&objects) {
			using BufferedNodeParser = nlohmann::adl_serializer<nv::BufferedNode>;
			objectGroupJson[BufferedNodeParser::typeCountKey<BufferedObject<T>*>()] = objects.size();
			objectRegionLengths.get<BufferedObject<T>*>() += (objects.size() * sizeof(BufferedObject<T>*));
			return nv::detail::STAY_IN_LOOP;
		}, objectGroup.objects);
		objectGroupNode.push_back(std::move(objectGroupJson));
	}
}

NodeSerializationResult nv::editor::createNodeJson(const std::vector<Layer>& layers, const ObjectGroupManager& objectGroups) 
{
	using namespace nv::detail::json_constants;

	json root;
	BufferedNode::TypeMap<size_t> objectRegionLengths{ 0 };

	writeObjectGroupData(root, objectGroups, objectRegionLengths);
	writeAllLayersData(root, layers, objectGroups, objectRegionLengths);
	auto byteCount = writeObjectSizeOffsetData(root, objectRegionLengths);

	return { root, byteCount };
}
