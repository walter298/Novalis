#pragma once

#include <bit>

namespace nv {
	namespace detail {
		template<typename T>
		void matchOffset(const std::byte* modelBuff, const T* modelPtr, std::byte* newBuff, T*& ptr) {
			auto srcEntryDist = reinterpret_cast<const std::byte*>(modelPtr) - modelBuff;
			ptr = reinterpret_cast<T*>(newBuff + srcEntryDist);
		}
	}
}
