#pragma once

#include <bit>
#include <functional>
#include <type_traits>

#include "../reflection/TemplateDetection.h"

namespace nv {
	namespace detail {
		template<typename Allocator, typename T>
		struct AllocatorHandle {
			Allocator allocator;

			using value_type = T;
			using pointer = T*;
			using const_pointer = const T*;
			using size_type = size_t;
			using difference_type = std::ptrdiff_t;
			template<typename U>
			struct rebind {
				using other = AllocatorHandle<Allocator, U>;
			};

			AllocatorHandle(const Allocator& allocator) : allocator{ allocator }
			{
			}
			AllocatorHandle(Allocator&& allocator) : allocator{ std::move(allocator) }
			{
			}

			template<typename U>
			AllocatorHandle(const AllocatorHandle<Allocator, U>& other) : allocator{ other.allocator }
			{
			}
			template<typename U>
			AllocatorHandle(AllocatorHandle<Allocator, U>&& other) : allocator{ std::move(other.allocator) }
			{
			}

			T* allocate(size_t n) {
				return unrefwrap(allocator).template allocate<T>(n);
			}

			void deallocate(T* p, size_t n) {
				unrefwrap(allocator).deallocate(p, n);
			}
		};
	}
}