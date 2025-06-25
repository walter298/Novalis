#include "../Point.h"

namespace nv {
	namespace physics {
		struct MovementTransform {
			Point dMove;
			float rotation = 0.0f; //in degrees
		};
	}
}