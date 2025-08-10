#include <cassert>
#include <cstddef>
#include <cstdlib>

//MSVC CRT does not support std::aligned_alloc, they make you use _aligned_malloc
#ifdef _MSC_VER
#define allocAlignedMemory(size, alignment) _aligned_malloc(size, alignment) //_aligned_malloc has opposite arg order
#define freeAlignedMemory _aligned_free
#else
#define allocAlignedMemory(size, alignment) std::aligned_alloc(alignment, size)
#define freeAlignedMemory std::free
#endif

namespace nv {
	namespace detail {
		template<typename T = std::byte, size_t Alignment = alignof(std::max_align_t)>
		struct AlignedBuffer {
		private:
			static_assert((Alignment & (Alignment - 1)) == 0, "Alignment must be a power of two!");

			static T* allocImpl(size_t n) {
				return static_cast<T*>(allocAlignedMemory(((n + Alignment - 1) / Alignment) * Alignment, Alignment));
			}
		public:
			T* data = nullptr;
		
			AlignedBuffer() noexcept = default;
			AlignedBuffer(size_t n) noexcept : data{ allocImpl(n) } {
				assert(data);
			}
			AlignedBuffer(const AlignedBuffer&) = delete;
			AlignedBuffer(AlignedBuffer&& other) noexcept {
				data = other.data;
				other.data = nullptr;
			}
			AlignedBuffer& operator=(size_t n) noexcept {
				if (data) {
					freeAlignedMemory(data);
				}
				data = allocImpl(n);
				return *this;
			}
			AlignedBuffer& operator=(AlignedBuffer&& other) noexcept {
				assert(other.data);
				data = other.data;
				other.data = nullptr;
				return *this;
			}

			~AlignedBuffer() noexcept {
				if (data) {
					freeAlignedMemory(data);
				}
			}
		};
	}
}