#pragma once

#include <tuple>

#include "ClassIteration.h"

namespace nv {
	namespace detail {
		namespace detail {
			//compiler is too dumb to deduce an nttp pack of 0 length in an integer sequence
			template<size_t currentTypeIdx, typename BuiltUpType, typename T>
			constexpr auto excludeMembers(BuiltUpType builtUp, T&& t, std::integer_sequence<size_t>) {
				if constexpr (currentTypeIdx == memberCount<T>()) {
					return builtUp;
				} else {
					return excludeMembers<currentTypeIdx + 1>(std::tuple_cat(builtUp, std::tie(std::get<currentTypeIdx>(t))), t, std::integer_sequence<size_t>{});
				}
			}

			template<size_t TypeIdx, bool Negation, template<typename> typename Pred, typename FilteredTuple, typename Tuple>
			constexpr auto filterDataMembersImpl(FilteredTuple filteredTuple, Tuple& tuple) {
				if constexpr (TypeIdx == memberCount<Tuple>()) {
					return filteredTuple;
				} else if constexpr (!Negation && Pred<GetType<TypeIdx, Tuple>>::value) {
					return filterDataMembersImpl<TypeIdx + 1, Negation, Pred>(
						std::tuple_cat(filteredTuple, std::tie(powerGet<TypeIdx>(tuple))), tuple
					);
				} else if constexpr (Negation && !Pred<GetType<TypeIdx, Tuple>>::value) {
					return filterDataMembersImpl<TypeIdx + 1, Negation, Pred>(
						std::tuple_cat(filteredTuple, std::tie(powerGet<TypeIdx>(tuple))), tuple
					);
				} else {
					return filterDataMembersImpl<TypeIdx + 1, Negation, Pred>(filteredTuple, tuple);
				}
			}
		}

		template<template<typename> typename Pred, bool Negation = false, typename T>
		constexpr auto filterDataMembers(T& t) {
			return detail::filterDataMembersImpl<0, Negation, Pred>(std::tuple{}, t);
		}
	}
}