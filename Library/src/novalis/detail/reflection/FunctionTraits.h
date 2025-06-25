#pragma once

#include <tuple>

namespace nv {
	namespace detail {
		template<typename T>
		struct FunctionTraits { //primary template assumes function call operator
			using Args = typename FunctionTraits<decltype(&T::operator())>::Args;
			using Ret = typename FunctionTraits<decltype(&T::operator())>::Ret;
			using Sig = typename FunctionTraits<decltype(&T::operator())>::Sig;
		};

		template<typename R, typename... Ts>
		struct FunctionTraits<R(Ts...)> { //specialization for functions that haven't decayed
			using Args = std::tuple<Ts...>;
			using Ret = R;
			using Sig = std::tuple<R, Ts...>;
		};

		template<typename R, typename... Ts>
		struct FunctionTraits<R(*)(Ts...)> { //specialization for function pointers
			using Args = std::tuple<Ts...>;
			using Ret = R;
			using Sig = std::tuple<R, Ts...>;
		};

		template<typename C, typename R, typename... Ts>
		struct FunctionTraits<R(C::*)(Ts...) const> { //specialization for const member functions
			using Args = std::tuple<Ts...>;
			using Ret = R;
			using Sig = std::tuple<R, Ts...>;
		};

		template<typename C, typename R, typename... Ts>
		struct FunctionTraits<R(C::*)(Ts...) const noexcept> { //specialization for const noexcept member functions
			using Args = std::tuple<Ts...>;
			using Ret = R;
			using Sig = std::tuple<R, Ts...>;
		};

		template<typename C, typename R, typename... Ts>
		struct FunctionTraits<R(C::*)(Ts...)> { //specialization for mutable member functions
			using Args = std::tuple<Ts...>;
			using Ret = R;
			using Sig = std::tuple<R, Ts...>;
		};

		template<typename C, typename R, typename... Ts>
		struct FunctionTraits<R(C::*)(Ts...) noexcept> { //specialization for noexcept mutable member functions
			using Args = std::tuple<Ts...>;
			using Ret = R;
			using Sig = std::tuple<R, Ts...>;
		};
	}
}