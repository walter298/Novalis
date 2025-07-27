#pragma once

#include <concepts>
#include <string>
#include <boost/functional/hash.hpp>

#include "detail/reflection/ClassIteration.h"

namespace nv {
	template<typename Object>
	class ID {
	public:
		using IntegerType = int64_t;
	private:
		IntegerType m_ID = 0;
	public:
		static inline IntegerType IDCount = 0;

		constexpr ID(IntegerType id) noexcept : m_ID(id) {}

		constexpr ID() noexcept {
			m_ID = IDCount;
			IDCount++;
		}

		static consteval ID None() noexcept {
			return ID{ -1 };
		}

		operator IntegerType() const noexcept {
			return m_ID;
		}

		MAKE_INTROSPECTION(m_ID)
	};
}

namespace boost {
	template<typename T>
	struct hash<nv::ID<T>> {
		size_t operator()(nv::ID<T> id) const {
			using Integer = typename nv::ID<T>::IntegerType;
			return boost::hash<Integer>()(id.operator Integer());
		}
	};
}

namespace std {
	template<typename T>
	struct hash<nv::ID<T>> {
		size_t operator()(nv::ID<T> id) const {
			using Integer = typename nv::ID<T>::IntegerType;
			return boost::hash<Integer>()(id.operator Integer());
		}
	};
}