#include "Arena.h"

void nv::Arena::initialize(size_t byteC) {
	arena = std::make_unique<std::byte[]>(byteC);
	capacity = byteC;
}

void* nv::Arena::alloc(size_t byteC) noexcept {
	assert(used + byteC < capacity);
	used += byteC;
	return arena.get() + used;
}

void nv::Arena::reset() noexcept {
	used = 0;
}

