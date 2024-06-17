#pragma once

#include <cassert>
#include <cstddef>
#include <memory>
#include <vector>
#include <plf_hive.h>

namespace nv {
	class Arena {
	private:
		static inline std::unique_ptr<std::byte[]> arena = nullptr;
		static inline size_t capacity = 0; //in bytes
		static inline size_t used = 0;
	public:
		static void initialize(size_t byteC);
		static void* alloc(size_t byteC) noexcept;
		static void reset() noexcept;
	};

	template<typename T>
	struct ArenaAllocator {
		using value_type = T;
		using size_type = std::size_t;
		using difference_type = std::ptrdiff_t;
	
		T* allocate(size_t byteC) {
			return reinterpret_cast<T*>(Arena::alloc(byteC));
		}

		void deallocate(T*, size_t) {}
	};

	template<typename T>
	using ArenaVector = std::vector<T, ArenaAllocator<T>>;

	template<typename T>
	using ArenaHive = plf::hive<T, ArenaAllocator<T>>;

	using ArenaString = std::basic_string<char, std::char_traits<char>, ArenaAllocator<char>>;
}