#pragma once

#include <cassert>
#include <memory>
#include <span>
#include <print>
#include <stacktrace>

namespace nv {
	namespace detail {
		class MemoryRegion {
		private:
			void* m_begin = nullptr;
			void* m_nextObjectBegin = nullptr;
			size_t m_capacity = 0;
			size_t m_space = 0;
		public:
			MemoryRegion() = default;
			MemoryRegion(void* begin, size_t capacity) noexcept
				: m_begin{ begin }, m_nextObjectBegin{ begin }, m_capacity{ capacity }, m_space{ capacity }
			{
			}

			template<typename T, typename... Args>
			T* emplace(Args&&... args) noexcept requires(std::is_nothrow_constructible_v<T, Args...>)
			{
				if (!std::align(alignof(T), sizeof(T), m_nextObjectBegin, m_space)) { //align ptr so we can construct a properly aligned object
					std::abort();
				}
				assert(m_space >= sizeof(T));
				std::memset(m_nextObjectBegin, 0, sizeof(T));
				T* obj = new (m_nextObjectBegin) T(std::forward<Args>(args)...);
				m_space -= sizeof(T);
				m_nextObjectBegin = static_cast<std::byte*>(m_nextObjectBegin) + sizeof(T);
				return obj;
			}

			inline std::byte* allocateBytes(size_t bytes, size_t alignment = sizeof(std::max_align_t)) noexcept {
				if (std::align(alignment, bytes, m_nextObjectBegin, m_space)) {
					auto temp = m_nextObjectBegin;
					m_nextObjectBegin = static_cast<std::byte*>(m_nextObjectBegin) + bytes;
					m_space -= bytes;
					return static_cast<std::byte*>(temp);
				} else {
					std::abort();
				}
			}

			template<typename T>
			inline T* allocate(size_t n) noexcept {
				assert(m_space % sizeof(T) == 0);
				//std::println("Allocating: {} {}", n, typeid(T).name());
				return reinterpret_cast<T*>(allocateBytes(n * sizeof(T), alignof(T)));
			}

			inline void deallocate(void* p, size_t n) noexcept {}

			inline void clear() noexcept {
				m_space = m_capacity;
				m_nextObjectBegin = m_begin;
			}

			inline size_t getTotalCapacity() const noexcept {
				return m_capacity;
			}

			size_t getSpace() const noexcept {
				return m_space;
			}

			inline size_t bytesAvailable() const noexcept {
				return m_space;
			}

			inline MemoryRegion makeSubregion(size_t offset, size_t len) noexcept {
				assert(m_capacity - offset >= len);
				return { static_cast<std::byte*>(m_begin) + offset, len };
			}
		};
	}
}