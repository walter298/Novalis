//#include "NodeLayout.h"
//
//template<typename Object>
//static constexpr void roundUpToNearestAlignment(size_t& n) noexcept {
//	constexpr size_t alignment = std::same_as<Object, std::byte> ? alignof(std::max_align_t) : alignof(Object);
//	n = (n + alignment - 1) & ~(alignment - 1);
//}
//
//static constexpr void roundUpToNearestAlignment(size_t& n, size_t alignment) noexcept {
//	n = (n + alignment - 1) & ~(alignment - 1);
//}
//
//void nv::editor::NodeLayout::resize(FileID nodeId, size_t newByteCount) noexcept {
//	auto nodeIt = nodes.find(nodeId);
//	assert(nodeIt != nodes.end());
//
//	auto& [resizedNodeOffset, resizedNodeSize] = nodeIt->second;
//	resizedNodeSize = newByteCount;
//
//	auto currOffset = resizedNodeOffset + newByteCount;
//	for (auto nextIt = std::next(nodeIt); nextIt != nodes.end(); nextIt++) {
//		roundUpToNearestAlignment<std::byte>(currOffset);
//		auto& [nodeOffset, nodeSize] = nextIt->second;
//		nodeOffset = currOffset;
//		currOffset += nodeSize;
//	}
//
//	if (currOffset > childNodeByteCount) {
//		totalByteCount += (currOffset - childNodeByteCount);
//	} else {
//		totalByteCount -= (childNodeByteCount - currOffset);
//	}
//	childNodeByteCount = currOffset;
//}