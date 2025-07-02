#pragma once

#include <array>
#include <algorithm>
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
			static constexpr bool containsKey() noexcept {
				return ((std::same_as<Key, Keys>) || ...);
			}

			template<typename Key>
			constexpr decltype(auto) get(this auto&& self) noexcept {
				static_assert(std::disjunction_v<std::is_same<Key, Keys>...>);
				static constexpr auto typeIdx = getTypeIndex<Key, 0, Keys...>();
				return self.m_values[typeIdx];
			}

			constexpr decltype(auto) getLast(this auto&& self) noexcept {
				return self.m_values[sizeof...(Keys) - 1];
			}
		private:
			template<template<typename> typename Filter, size_t Idx, typename Func>
			constexpr void forEachImpl(this auto&& self, Func f) {
				using ObjectType = std::tuple_element_t<Idx, std::tuple<Keys...>>;
				if constexpr (Filter<ObjectType>::value) {
					f.operator()<ObjectType>(self.m_values[Idx]);
				}
			}

			template<template<typename> typename Filter, typename Func, size_t... Idxs>
			constexpr void forEachImpl(this auto&& self, Func f, std::index_sequence<Idxs...> idxs) {
				((self.forEachImpl<Filter, Idxs>(f)), ...);
			}
		public:
			template<typename Func>
			constexpr void forEach(this auto&& self, Func f) {
				self.forEachImpl<NoFilter>(f, std::make_index_sequence<sizeof...(Keys)>{});
			}
			template<template<typename> typename Filter, typename Func>
			constexpr void forEachFiltered(this auto&& self, Func f) {
				self.forEachImpl<Filter>(f, std::make_index_sequence<sizeof...(Keys)>{});
			}
		private:
			template<size_t N, typename Func>
			constexpr static void zipImpl(TypeMap& first, TypeMap& second, Func f) {
				using ObjectType = std::tuple_element_t<N, std::tuple<Keys...>>;
				f.operator()<ObjectType>(first.m_values[N], second.m_values[N]);
			};

			template<typename Func, size_t... Idxs>
			static constexpr void forEachZipImpl(TypeMap& first, TypeMap& second, Func f, std::index_sequence<Idxs...>) {
				((zipImpl<Idxs>(first, second, f)), ...);
			}
		public:
			template<typename Func>
			static constexpr void forEachZip(TypeMap& first, TypeMap& second, Func f) {
				forEachZipImpl(first, second, f, std::make_index_sequence<sizeof...(Keys)>{});
			}

			bool operator==(const TypeMap& other) const noexcept {
				return std::ranges::equal(other.m_values, m_values);
			}

		};
	}
}