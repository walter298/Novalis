#include "../LineSegment.h"

namespace nv {
	namespace physics {
		struct CollisionData {
			Point mtv;
			LineSegment collidedStaticSegment;
		};
	}
}