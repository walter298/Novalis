#include "Node.h"

using ObjectLookups = nv::BufferedNode::ObjectLookups;
using Layer = nv::BufferedNode::Layer;
using Layers = nv::BufferedNode::Layers;

void copyLookupMaps(const std::byte* srcArena, std::byte* destArena, 
	const ObjectLookups& srcLookups, ObjectLookups& destLookups) 
{
	nv::detail::forEachDataMember([&](const auto& srcMap, auto& destMap) {
		using Map = std::remove_cvref_t<decltype(srcMap)>;
		Map::copy(srcArena, destArena, srcMap, destMap);
		return nv::detail::STAY_IN_LOOP;
	}, srcLookups, destLookups);
}

void nv::BufferedNode::copyNodeSpan(const std::byte* srcArena, std::byte* destArena, 
	const std::span<BufferedNode*>& srcSpan, std::span<BufferedNode*>& destSpan)
{
	for (auto [srcNode, destNode] : std::views::zip(srcSpan, destSpan)) {
		//makes dest node point to the same relative address as the source node
		nv::detail::matchOffset(srcArena, srcNode, destArena, destNode);

		//make the dest node's arena point to the same relative address as the source node's arena
		auto& srcChildArena  = std::get<std::byte*>(srcNode->m_arena);
		auto& destChildArena = std::get<std::byte*>(destNode->m_arena);
		nv::detail::matchOffset(srcArena, srcChildArena, destArena, destChildArena);

		deepCopyChild(*srcNode, std::get<std::byte*>(srcNode->m_arena), *destNode, destArena);
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
	//make dest layers point to same relative address 
	Layer* destLayersData = nullptr;
	detail::matchOffset(srcArena, srcLayers.data(), destArena, destLayersData);
	destLayers = { destLayersData, srcLayers.size() };

	for (auto [srcLayer, destLayer] : std::views::zip(srcLayers, destLayers)) {
		deepCopyLayer(srcArena, destArena, srcLayer, destLayer);
		int k = 0;
	}
}

void nv::BufferedNode::deepCopyChild(const nv::BufferedNode& src, const std::byte* srcArena,
	nv::BufferedNode& dest, std::byte* destArena)
{
	copyLayers(srcArena, destArena, src.m_objectLayers, dest.m_objectLayers);
	copyLookupMaps(srcArena, destArena, src.m_objectLookups, dest.m_objectLookups);
}

nv::BufferedNode::BufferedNode(const BufferedNode& other) {
	m_arena = std::make_unique<std::byte[]>(other.m_byteC);

	std::byte* srcArena = nullptr;
	if (std::holds_alternative<std::unique_ptr<std::byte[]>>(other.m_arena)) {
		srcArena = std::get<std::unique_ptr<std::byte[]>>(other.m_arena).get();
	} else {
		srcArena = std::get<std::byte*>(other.m_arena);
	}
	deepCopyChild(other, srcArena, *this, std::get<std::unique_ptr<std::byte[]>>(m_arena).get());
}