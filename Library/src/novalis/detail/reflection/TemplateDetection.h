#pragma once

#include <type_traits>

namespace nv {
	namespace detail {
		template<template<typename...> typename ClassTemplate, typename>
		struct IsClassTemplate : public std::false_type {};

		template<template<typename...> typename ClassTemplate, typename... Ts>
		struct IsClassTemplate<ClassTemplate, ClassTemplate<Ts...>> : public std::true_type {};

		//static_assert(IsClassTemplate<std::tuple, std::tuple<int, int, int>>::value);

		template<typename T>
		constexpr auto& unrefwrap(T& t) {
			if constexpr (IsClassTemplate<std::reference_wrapper, std::remove_cvref_t<T>>::value) {
				return t.get();
			} else {
				return t;
			}
		}

		template<typename T>
		constexpr auto& unptrwrap(T& t) {
			if constexpr (IsClassTemplate<std::unique_ptr, T>::value || std::is_pointer_v<T>) {
				return *t;
			} else {
				return t;
			}
		}

		template<template<typename... Ts> typename T, typename... Ts>
		struct GetParameterizedTypeFromTuple {};

		template<template<typename... Ts> typename Parameterized, typename... Ts2>
		struct GetParameterizedTypeFromTuple<Parameterized, std::tuple<Ts2...>> {
			using Type = Parameterized<Ts2...>;
		};

		template<typename Parameterized>
		struct GetTemplateTypes {};

		template<template<typename... GTs> typename Parameterized, typename... Ts>
		struct GetTemplateTypes<Parameterized<Ts...>> {
			using Types = std::tuple<Ts...>;
		};
	}
}