//#pragma once
//
//#include <cassert>
//#include <span>
//
//template<typename T>
//class BufferViewAllocator {
//private:
//	T* m_buff;
//	size_t m_idx = 0;
//public:
//	BufferViewAllocator(T* buff) noexcept
//		: m_buff{ buff }
//	{
//	}
//
//	template<typename U>
//	friend class ArenaAllocator;
//
//	template<typename U> //rebinding constructor
//	BufferViewAllocator(const BufferViewAllocator<U>& other)
//		: m_buff{ other.m_buff }
//	{
//	};
//
//	//allocator boilerplate
//	using size_type = size_t;
//	using value_type = T;
//	using difference_type = std::ptrdiff_t;
//	template <typename U>
//	struct rebind {
//		using other = BufferViewAllocator<U>;
//	};
//
//	T* allocate(size_t n) noexcept {
//		auto ptr = static_cast<T*>(m_arena.get().allocate(n * sizeof(T), alignof(T)));
//		assert(ptr != nullptr);
//		return ptr;
//	}
//	void deallocate(T*, size_t) noexcept {} //no-op
//};