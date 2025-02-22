#pragma once

#include <cassert>
#include <concepts>
#include <memory>
#include <vector>
#include <boost/smart_ptr/allocate_unique.hpp>

#include "Reflection.h"
#include "SharedBuffer.h"

namespace nv {
	class Arena {
	private:
		SharedBuffer m_buff;
		void* m_nextObjectBegin;
		size_t m_capacity;
		size_t m_space;
	public:
		Arena(size_t size)  
			: m_buff{ size }, m_nextObjectBegin{ m_buff.data() }, m_capacity{ size }, m_space{ size }
		{
		}

		template<typename Allocator>
		Arena(size_t size, Allocator& allocator)
			: m_buff{ allocator, size }, 
			m_nextObjectBegin{ m_buff.data() }, m_capacity{ size }, m_space{ size }
		{
		}

		template<typename T, typename... Args>
		T* emplace(Args&&... args) noexcept 
			requires(std::is_nothrow_constructible_v<T, Args...> && std::is_trivially_destructible_v<T>) 
		{
			if (!std::align(alignof(T), sizeof(T), m_nextObjectBegin, m_space)) { //align ptr so we can construct a properly aligned object
				return nullptr;
			}
			T* obj = new (m_nextObjectBegin) T(std::forward<Args>(args)...);
			m_nextObjectBegin = static_cast<std::byte*>(m_nextObjectBegin) + sizeof(T);
			return obj;
		}

		void* allocate(size_t bytes, size_t alignment = sizeof(std::max_align_t)) noexcept {
			if (std::align(alignment, bytes, m_nextObjectBegin, m_space)) {
				auto temp = m_nextObjectBegin;
				m_nextObjectBegin = static_cast<std::byte*>(m_nextObjectBegin) + bytes;
				return temp;
			} else {
				return nullptr;
			}
		}

		void clear() noexcept {
			m_space = m_capacity;
			m_nextObjectBegin = m_buff.data();
		}
	};

	template<typename T>
	class ArenaAllocator {
	private:
		std::reference_wrapper<Arena> m_arena;
	public:
		ArenaAllocator(Arena& arena) noexcept
			: m_arena{ arena }
		{
		}

		template<typename U>
		friend class ArenaAllocator;

		template<typename U> //rebinding constructor
		ArenaAllocator(const ArenaAllocator<U>& other) 
			: m_arena{ other.m_arena }
		{
		};

		//allocator boilerplate
		using size_type = size_t;
		using value_type = T;
		using difference_type = std::ptrdiff_t;
		template <typename U>
		struct rebind {
			using other = ArenaAllocator<U>;
		};

		T* allocate(size_t n) noexcept {
			auto ptr = static_cast<T*>(m_arena.get().allocate(n * sizeof(T), alignof(T)));
			assert(ptr != nullptr);
			return ptr;
		}
		void deallocate(T*, size_t) noexcept {} //no-op
	};
}