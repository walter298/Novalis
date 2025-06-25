#pragma once

#include <bitset>
#include <functional>

#include <boost/unordered/unordered_flat_map.hpp>

#include <SDL3/SDL_scancode.h>
#include <SDL3/SDL_keyboard.h>

#include "detail/reflection/FunctionTraits.h"

namespace nv {
	template<typename Ret, typename... Args> 
	using Event = std::move_only_function<Ret(Args...)>;

	enum class MouseButtonState {
		Down,
		Released,
		None
	};
	
	struct MouseData {
		MouseButtonState left = MouseButtonState::None;
		MouseButtonState mid = MouseButtonState::None;
		MouseButtonState right = MouseButtonState::None;
		float x = 0.0f;
		float y = 0.0f;
		float deltaX = 0.0f;
		float deltaY = 0.0f;
	};

	using MouseEvent    = Event<void, MouseData>;
	using KeyboardEvent = Event<void, const Uint8*>;
}
