#include "BufferedNode.h"

using ObjectLookups = nv::BufferedNode::ObjectLookups;
using Layer = nv::BufferedNode::Layer;
using Layers = nv::BufferedNode::Layers;

template<typename T>
using PtrMap = nv::detail::BufferedNodeTraits::ObjectLookupMap<nv::detail::BufferedString, T*>;

template<typename T>
static void copyPtrMap(const std::byte* srcArena, std::byte* destArena,
	const PtrMap<T>& srcMap, PtrMap<T>& destMap) 
{
	std::println("Copying PtrMap with size: {}", srcMap.arr.size());

	//make dest map's array point to the same relative address as the src map's array
	typename PtrMap<T>::Entry* destMapPtr = nullptr;
	nv::detail::matchOffset(srcArena, srcMap.arr.data(), destArena, destMapPtr);
	destMap.arr = { destMapPtr, srcMap.arr.size() }; //set pointer and size of destMap

	//copy each entry from the src map into the dest map
	for (auto&& [srcEntry, destEntry] : std::views::zip(srcMap, destMap)) {
		const auto& [srcName, srcObject, srcIsTombstone] = srcEntry;
		auto& [destName, destObject, destIsTombstone] = destEntry;

		destName = srcName.copy(srcArena, destArena); //copy the name string
		nv::detail::matchOffset(srcArena, srcObject, destArena, destObject); //make destObject point to the same relative address as srcObject
		destIsTombstone = srcIsTombstone; //copy the tombstone flag
	}
}

static void copyLookupMaps(const std::byte* srcArena, std::byte* destArena, 
	const ObjectLookups& srcLookups, ObjectLookups& destLookups) 
{
	nv::detail::forEachDataMember([&](const auto& srcMap, auto& destMap) {
		copyPtrMap(srcArena, destArena, srcMap, destMap);
		return nv::detail::STAY_IN_LOOP;
	}, srcLookups, destLookups);
}

using ObjectGroup = nv::detail::BufferedNodeTraits::ObjectGroup;

static ObjectGroup copyObjectGroup(const std::byte* srcArena, std::byte* destArena, const ObjectGroup& srcObjectGroup) {
	ObjectGroup destObjectGroup;
	nv::detail::forEachDataMember([&]<typename Object>(const std::span<Object*>& srcObjectPtrSpan,
		std::span<Object*>& destObjectPtrSpan) 
	{
		if (!srcObjectPtrSpan.data() || srcObjectPtrSpan.empty()) {
			destObjectPtrSpan = std::span<Object*>{};
			return nv::detail::STAY_IN_LOOP;
		}
		//make dest object ptr span point to the same relative address as the src ptr object span
		Object** destSpanPtr = nullptr;
		nv::detail::matchOffset(srcArena, srcObjectPtrSpan.data(), destArena, destSpanPtr);
		destObjectPtrSpan = { destSpanPtr, srcObjectPtrSpan.size() };

		for (auto&& [srcPtr, destPtr] : std::views::zip(srcObjectPtrSpan, destObjectPtrSpan)) {
			//make dest object ptr point to the same relative address as the src object ptr
			nv::detail::matchOffset(srcArena, srcPtr, destArena, destPtr);
		}
		return nv::detail::STAY_IN_LOOP;
	}, srcObjectGroup, destObjectGroup);
	return destObjectGroup;
}

void nv::BufferedNode::copyGroupMaps(const std::byte* srcArena, std::byte* destArena,
	const ObjectGroupMap& srcObjectGroupMap, ObjectGroupMap& destObjectGroupMap)
{
	//make dest map's array point to the same relative address as the src map's array
	ObjectGroupMap::Entry* destMapPtr = nullptr;
	nv::detail::matchOffset(srcArena, srcObjectGroupMap.arr.data(), destArena, destMapPtr);
	destObjectGroupMap.arr = { destMapPtr, srcObjectGroupMap.arr.size() }; //set pointer and size of destObjectGroupMap

	for (const auto& [srcEntry, destEntry] : std::views::zip(srcObjectGroupMap, destObjectGroupMap)) {
		const auto& [srcName, srcObjectGroup, srcTombstone] = srcEntry;
		auto& [destName, destObjectGroup, destIsTombstone] = destEntry;
		destName = srcName.copy(srcArena, destArena); //copy the name string
		destObjectGroup = copyObjectGroup(srcArena, destArena, srcObjectGroup); //copy the object map
		destIsTombstone = srcTombstone; //copy the tombstone flag
	}
}

void nv::BufferedNode::copyNodeSpan(const std::byte* srcParentArena, std::byte* destParentArena,
	const std::span<BufferedNode*>& srcSpan, std::span<BufferedNode*>& destSpan)
{
	for (auto [srcNodePtr, destNodePtr] : std::views::zip(srcSpan, destSpan)) {
		//makes dest node point to the same relative address as the source node
		nv::detail::matchOffset(srcParentArena, srcNodePtr, destParentArena, destNodePtr);

		destNodePtr = new (destNodePtr) BufferedNode{}; //construct the dest node in place

		destNodePtr->m_byteC = srcNodePtr->m_byteC; //copy the byte count

		//make the dest node's arena point to the same relative address as the source node's arena
		auto& srcChildArena  = std::get<std::byte*>(srcNodePtr->m_arena);
		auto& destChildArena = std::get<std::byte*>(destNodePtr->m_arena);
		nv::detail::matchOffset(srcParentArena, srcChildArena, destParentArena, destChildArena);

		deepCopyChild(*srcNodePtr, srcChildArena, *destNodePtr, destChildArena);
	}
}

void nv::BufferedNode::deepCopyLayer(const std::byte* srcArena, std::byte* destArena, 
	const Layer& srcLayer, Layer& destLayer)
{
	nv::detail::forEachDataMember([&](const auto& srcObjectSpan, auto& destObjectSpan) {
		deepCopySpan(srcArena, destArena, srcObjectSpan, destObjectSpan);
		return nv::detail::STAY_IN_LOOP;
	}, srcLayer, destLayer);
}

void nv::BufferedNode::copyLayers(const std::byte* srcArena, std::byte* destArena, 
	const Layers& srcLayers, Layers& destLayers)
{
	//make dest layers point to same relative address as the src layers
	Layer* destLayersData = nullptr; 
	detail::matchOffset(srcArena, srcLayers.data(), destArena, destLayersData);

	destLayers = { destLayersData, srcLayers.size() }; //set pointer and size of destLayers

	for (auto [srcLayer, destLayer] : std::views::zip(srcLayers, destLayers)) {
		deepCopyLayer(srcArena, destArena, srcLayer, destLayer);
	}
}

void nv::BufferedNode::deepCopyChild(const nv::BufferedNode& src, const std::byte* srcArena,
	nv::BufferedNode& dest, std::byte* destArena)
{
	copyTrivialBaseMembers(dest, src);                                               //copy trivial members
	copyLayers(srcArena, destArena, src.m_objectLayers, dest.m_objectLayers);        //copy layers
	copyLookupMaps(srcArena, destArena, src.m_objectLookups, dest.m_objectLookups);  //copy object maps
	copyGroupMaps(srcArena, destArena, src.m_objectGroupMap, dest.m_objectGroupMap); //copy object group maps
	copyPtrMap(srcArena, destArena, src.m_layerMap, dest.m_layerMap);                //copy layer map
}

nv::BufferedNode::~BufferedNode() noexcept {
	if (!m_movedFrom) {
		this->forEach([]<typename Object>(auto layer, Object& object) {
			object.~Object();
			return nv::detail::STAY_IN_LOOP;
		});
	}
}

nv::BufferedNode::BufferedNode(const BufferedNode& other) 
	: m_byteC{ other.m_byteC }, m_arena{ detail::AlignedBuffer<std::byte>{ other.m_byteC }
}
{
	std::byte* srcArena = nullptr;
	if (std::holds_alternative<detail::AlignedBuffer<std::byte>>(other.m_arena)) {
		srcArena = std::get<detail::AlignedBuffer<std::byte>>(other.m_arena).data;
	} else {
		srcArena = std::get<std::byte*>(other.m_arena);
	}
	deepCopyChild(other, srcArena, *this, std::get<detail::AlignedBuffer<std::byte>>(m_arena).data);
}

nv::BufferedNode::BufferedNode(BufferedNode&& other) noexcept :
	m_arena{ std::move(other.m_arena) }, m_byteC{ other.m_byteC }
{
	//assign base members
	moveStorageBaseMembers(*this, other);
	copyTrivialBaseMembers(*this, other);

	other.m_movedFrom = true;
}
