#pragma once

#include "../../Node.h"

namespace nv {
	namespace detail {
		template<typename T>
		struct ByteSizeCalculator {
			template<bool IsBase>
			static void calculate(const T& t, size_t& n) noexcept {
				n += (IsBase ? sizeof(T) : 0);
				detail::forEachDataMember([&](const auto& field) {
					using FieldType = std::remove_cvref_t<decltype(field)>;
					if constexpr (!concepts::Primitive<FieldType>) {
						ByteSizeCalculator<FieldType>::calculate(field, n);
					}
					return STAY_IN_LOOP;
				}, t);
			}
		};

		template<typename T> requires(std::ranges::viewable_range<T>)
		struct ByteSizeCalculator {
			template<bool IsBase>
			static void calculate(const T& t, size_t& n) {

			}
		};

		template<bool IsBase = true, typename T>
		static constexpr void calculateSizeBytes(const T& t, BufferedNode::TypeMap<size_t>& objectRegionLengths) {
			if (std::same_as<std::remove_cvref_t<T>, BufferedNode>) {
				//todo
			} else if constexpr (concepts::Primitive<T>) {
				objectRegionLengths.get<T>() += sizeof(T);
			}
			else if constexpr (std::ranges::viewable_range<T>) {
				using ValueType = typename T::value_type;
				if constexpr (concepts::Primitive<ValueType>) {
					objectRegionLengths += (sizeof(ValueType) * std::ranges::size(t));
				} else {
					return std::accumulate(std::begin(t), std::end(t), 0, [](size_t bytes, const auto& elem) {
						return bytes + calculateSizeBytes(elem, objectRegionLengths);
					});
				}
			}
			else {
				//objectRegionLengths.get<T>() += (IsBase ? sizeof(T) : 0);
				
			}
		}
	}
}