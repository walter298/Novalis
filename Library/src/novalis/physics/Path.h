#pragma once

#include <optional>
#include "../EventHandler.h"
#include "../Point.h"

namespace nv {
	namespace physics {
		template<typename T>
		concept Path = requires(T t) {
			{ t.move() } -> std::same_as<std::optional<Point>>;
		};

		template<typename Object, Path Path>
		auto makePath(Object& object, Path path) {
			return [&object, path = std::move(path)]() mutable -> bool {
				auto delta = path.move();
				if (!delta) {
					return EventHandler::END_EVENT;
				}
				constexpr auto MIN_LENGTH = 0.0001f;
				if (delta->calcMagnitude() < MIN_LENGTH) {
					return EventHandler::END_EVENT;
				}
				object.move(*delta);
				return EventHandler::CONTINUE_EVENT;
			};
		}
	}
}