#pragma once

#include <tuple>
#include <boost/describe.hpp>
#include <boost/mp11.hpp>
#include <boost/pfr.hpp>
#include <boost/preprocessor.hpp>

#include "TemplateDetection.h"

namespace nv {
	namespace detail {
		namespace pfr = boost::pfr;

		struct GetMembers {
			template<size_t N, typename T>
			static constexpr decltype(auto) get(T& t) {
				return std::get<N>(t.__makeMemberTuple());
			}

			template<typename T>
			static constexpr size_t memberCount() noexcept {
				return T::__MemberCount;
			}
		};

#define ADD_DECLTYPE(r, data, elem) std::remove_cvref_t<decltype(elem)>
#define ADD_DECLTYPE_ARGS(...) BOOST_PP_SEQ_ENUM(BOOST_PP_SEQ_TRANSFORM(ADD_DECLTYPE, _, BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__)))

#define PREFIX_SELF(r, data, elem) self.elem
#define PREFIX_SELF_ARGS(...) BOOST_PP_SEQ_ENUM(BOOST_PP_SEQ_TRANSFORM(PREFIX_SELF, _, BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__)))

#define MAKE_INTROSPECTION(...) \
friend struct nv::detail::GetMembers; \
private: \
auto __makeMemberTuple(this auto&& self) { \
	return std::tie(PREFIX_SELF_ARGS(__VA_ARGS__)); \
} \
public:\
static constexpr size_t __MemberCount = BOOST_PP_VARIADIC_SIZE(__VA_ARGS__);\
using __Tuple = std::tuple<ADD_DECLTYPE_ARGS(__VA_ARGS__)>; 

		template<typename T>
		concept MemberIterable = T::__MemberCount > 0;

		//get that works with tuples, aggregates, and classes
		template<size_t Idx, typename T>
		constexpr decltype(auto) powerGet(T& t) {
			if constexpr (MemberIterable<std::remove_cvref_t<T>>) {
				return GetMembers::template get<Idx>(t); //introspective class case
			} else if constexpr (std::is_aggregate_v<std::remove_cvref_t<T>> && !MemberIterable<std::remove_cvref_t<T>>) {
				return pfr::get<Idx>(t); //aggregate case
			} else if constexpr (IsClassTemplate<std::tuple, std::remove_cvref_t<T>>::value) {
				return std::get<Idx>(t); //tuple case
			} 
		}

		namespace detail {
			template<bool B, size_t Idx, typename T>
			struct GetTypeImpl {
				using type = std::tuple_element_t<Idx, std::remove_cvref_t<T>>;
			};

			template<size_t Idx, typename T> requires (std::is_aggregate_v<T>)
			struct GetTypeImpl<true, Idx, T> {
				using type = pfr::tuple_element_t<Idx, std::remove_cvref_t<T>>;
			};
		}

		template<size_t Idx, typename T>
		using GetType = typename detail::GetTypeImpl<std::is_aggregate_v<std::remove_cvref<T>>, Idx, T>::type;

		template<typename T>
		consteval size_t memberCount() {
			using Plain = std::remove_cvref_t<T>;
			if constexpr (MemberIterable<Plain>) {
				return Plain::__MemberCount;
			} else if constexpr (std::is_aggregate_v<Plain>) {
				return pfr::tuple_size_v<Plain>; //aggregate case
			} else if constexpr (IsClassTemplate<std::tuple, Plain>::value) {
				return std::tuple_size_v<Plain>; //tuple case
			} 
		}

		namespace detail {
			template<size_t MemberIdx, typename Func, typename TiedStructs, size_t... StructIdxs>
			constexpr bool iterateStructMembers(Func f, TiedStructs tiedStructs, std::index_sequence<StructIdxs...>) {
				auto tiedMembers = std::tie(powerGet<MemberIdx>(powerGet<StructIdxs>(tiedStructs))...);
				return std::apply(f, tiedMembers);
			}

			template<typename Func, typename TiedStructs, size_t... MemberIdxs>
			constexpr bool iterateStructsImpl(Func f, TiedStructs tiedStructs, std::index_sequence<MemberIdxs...>) {
				return ((iterateStructMembers<MemberIdxs>(f, tiedStructs, std::make_index_sequence<memberCount<TiedStructs>()>())) || ...);
			}
		}

		template<typename Func, typename FirstStruct, typename... Structs> //FirstStruct param lets us extract member count of each tuple (assumes all tuples have same count of members)
		constexpr bool forEachDataMember(Func f, FirstStruct& firstStruct, Structs&... structs) {
			return detail::iterateStructsImpl(f, std::tie(firstStruct, structs...), std::make_index_sequence<memberCount<FirstStruct>()>());
		}

		inline constexpr bool STAY_IN_LOOP = false;
		inline constexpr bool BREAK_FROM_LOOP = true;
	}
}