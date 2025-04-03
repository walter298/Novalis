#pragma once

#include <queue>
#include <plf_hive.h>

#include "MemoryRegion.h"

namespace nv {
	namespace detail {
		/*class MemoryPool;

		class MemoryPoolEntry {
		private:
			size_t m_blockIdx;
		public:
			MemoryRegion region;
			MemoryPoolEntry(size_t blockIdx, MemoryRegion region) noexcept
				: m_blockIdx{ blockIdx }, region{ std::move(region) }
			{
			}
			~MemoryPoolEntry() noexcept;

			friend class MemoryPool;
		};

		class MemoryPool {
		private:
			struct Block {
				std::unique_ptr<std::byte[]> buff;
				MemoryRegion allocator;
			};
			std::vector<Block> m_blocks;

			struct SpaceComparator {
				bool operator()(const MemoryPoolEntry& a, const MemoryPoolEntry& b) const noexcept {
					return a.region.bytesAvailable() > b.region.bytesAvailable();
				}
			};
			using RegionQueue = std::priority_queue<MemoryPoolEntry, std::vector<MemoryPoolEntry>, SpaceComparator>;
			RegionQueue m_mostEmptyRegions;
		public:
			void makeRegionReusable(MemoryPoolEntry&& entry) {
				m_mostEmptyRegions.push(std::move(entry));
			}
		private:
			MemoryRegion allocateNewRegion(size_t byteC) {

			}
		public:
			MemoryRegion allocateRegion(size_t byteC) {
				assert()
				if (m_mostEmptyRegions.empty()) {
					
				}
			}
		};*/
	}
}