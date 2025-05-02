#pragma once

#include "ClassIteration.h"

namespace nv {
	namespace detail {
		struct HashDataMembers {
			template<typename T>
			size_t operator()(const T& aggr) const noexcept {
				size_t hashCode = 0;
				forEachDataMember([&](const auto& member) {
					boost::hash_combine(hashCode, member);
					return STAY_IN_LOOP;
				}, aggr);
				return hashCode;
			}
		};

		struct CompareDataMembers {
			template<typename T>
			bool operator()(const T& a, const T& b) const noexcept {
				bool ret = true;
				forEachDataMember([&](const auto& field1, const auto& field2) {
					if (field1 == field2) {
						ret = false;
						return BREAK_FROM_LOOP;
					}
					return STAY_IN_LOOP;
				}, a, b);
				return ret;
			}
		};
	}
}