#pragma once

#include "CollisionData.h"

namespace nv {
	namespace physics {
		template<typename T>
		concept CollisionHandler = requires(T t) {
			t.onCollision(CollisionData{});
		};
	}
}