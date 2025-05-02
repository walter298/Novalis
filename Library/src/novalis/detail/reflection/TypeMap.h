#pragma once

#include <array>
#include <tuple>

#include "TypeInfo.h"

namespace nv {
	namespace detail {
		template<typename T>
		struct NoFilter : public std::true_type {};

		template<typename Value, typename... Keys>
		class TypeMap {
		private:
			std::array<Value, sizeof...(Keys)> m_values;

			template<typename KeyTarget, size_t I, typename IthKey, typename... OtherKeys>
			static consteval size_t getTypeIndex() {
				static_assert(I < sizeof...(Keys), "TypeMap key not found");
				if constexpr (std::same_as<KeyTarget, IthKey>) {
					return I;
				} else {
					return getTypeIndex<KeyTarget, I + 1, OtherKeys...>();
				}
			}
		public:
			constexpr TypeMap(Value v) {
				std::ranges::fill(m_values, v);
			}
			constexpr TypeMap() = default;

			template<typename Key>
			constexpr bool contains() const noexcept {
				return ((std::same_as<Key, Keys>) || ...);
			}

			template<typename Key>
			constexpr decltype(auto) get(this auto&& self) noexcept {
				static constexpr auto typeIdx = getTypeIndex<Key, 0, Keys...>();
				return self.m_values[typeIdx];
			}

			constexpr decltype(auto) getLast(this auto&& self) noexcept {
				return self.m_values[sizeof...(Keys) - 1];
			}
		private:
			template<template<typename> typename Filter, size_t Idx, typename Func>
			void forEachImpl(this auto&& self, Func f) {
				using ObjectType = std::tuple_element_t<Idx, std::tuple<Keys...>>;
				if constexpr (Filter<ObjectType>::value) {
					f.operator()<ObjectType>(self.m_values[Idx]);
				}
			}

			template<template<typename> typename Filter, typename Func, size_t... Idxs>
			void forEachImpl(this auto&& self, Func f, std::index_sequence<Idxs...> idxs) {
				((self.forEachImpl<Filter, Idxs>(f)), ...);
			}
		public:
			template<typename Func>
			void forEach(this auto&& self, Func f) {
				self.forEachImpl<NoFilter>(f, std::make_index_sequence<sizeof...(Keys)>{});
			}
			template<template<typename> typename Filter, typename Func>
			void forEachFiltered(this auto&& self, Func f) {
				self.forEachImpl<Filter>(f, std::make_index_sequence<sizeof...(Keys)>{});
			}
		};
	}
}