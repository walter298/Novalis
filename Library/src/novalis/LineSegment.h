#pragma once

#include <boost/geometry.hpp>

#include "Point.h"

namespace nv {
	using LineSegment = boost::geometry::model::segment<Point>;
}