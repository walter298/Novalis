#include "Intersection.h"

#include <print>

#include "../LineSegment.h"

struct Projection {
	float min = std::numeric_limits<float>::max();
	float max = std::numeric_limits<float>::lowest();
};

static bool overlaps(Projection a, Projection b) {
	return a.min <= b.max && b.min <= a.max;
}

static Projection getProjectionExtrema(const std::span<nv::Point>& points, nv::Point unitAxis) noexcept {
	Projection extrema;
	for (const auto& point : points) {
		auto dot = boost::geometry::dot_product(point, unitAxis);
		if (dot < extrema.min) {
			extrema.min = dot;
		}
		if (dot > extrema.max) {
			extrema.max = dot;
		}
	}
	return extrema;
}

bool nv::physics::intersects(const std::span<Point>& aPoints, const std::span<Point>& bPoints) noexcept {
	auto projectOntoEachPerpAxis = [&](const std::span<Point>& points) {
		assert(points.size() > 1);
		for (size_t i = 0; i < points.size(); i++) {
			auto p1 = points[i];
			auto p2 = points[(i + 1) % points.size()];
			if (p1 == p2) { //skip worthless edges
				continue;
			}

			auto direction = p2 - p1;
			Point perpAxis{ direction.y, -direction.x };
			auto unitPerpAxis = perpAxis.normalize();

			auto projectionA = getProjectionExtrema(aPoints, unitPerpAxis);
			auto projectionB = getProjectionExtrema(bPoints, unitPerpAxis);

			if (!overlaps(projectionA, projectionB)) { //polygons don't intersect
				return false; 
			}
		}

		return true;
	};
	return projectOntoEachPerpAxis(aPoints) && projectOntoEachPerpAxis(bPoints);
}

std::optional<nv::physics::CollisionData> nv::physics::calcCollisionData(
	const std::span<Point>& staticPoints, const std::span<Point>& movingPoints) noexcept
{
	auto projectOntoEachPerpAxis = [&](const std::span<Point>& points) -> std::optional<CollisionData> {
		Point mtv;
		auto mtvDistance = std::numeric_limits<float>::max();
		LineSegment seg;
		
		assert(points.size() > 1);

		Point staticCenter, movingCenter;
		boost::geometry::centroid(staticPoints, staticCenter);
		boost::geometry::centroid(movingPoints, movingCenter);

		for (size_t i = 0; i < points.size(); i++) {
			//get polygon edge
			auto p1 = points[i];
			auto p2 = points[(i + 1) % points.size()];
			if (p1 == p2) { //skip worthless edges
				continue;
			}

			//calculate normal vector to polygon edge
			auto direction = p2 - p1;
			Point perpAxis{ direction.y, -direction.x };
			auto unitPerpAxis = perpAxis.normalize();

			//ensure that the direction of the MTV is the same as the static polygon moving out in the direction of the moving one
			if (boost::geometry::dot_product(movingCenter - staticCenter, unitPerpAxis) < 0) {
				unitPerpAxis = -unitPerpAxis;
			}

			//the projections of both polygons onto the normal vector must overlap
			auto staticPolyProjection = getProjectionExtrema(staticPoints, unitPerpAxis);
			auto movingPolyProjection = getProjectionExtrema(movingPoints, unitPerpAxis);
			if (!overlaps(staticPolyProjection, movingPolyProjection)) { 
				return std::nullopt; //polygons don't intersect
			}

			//calculate the length of the overlap
			auto overlapDist = std::min(staticPolyProjection.max, movingPolyProjection.max) - 
							   std::max(staticPolyProjection.min, movingPolyProjection.min);
			if (overlapDist < mtvDistance) {
				mtv = unitPerpAxis * overlapDist;
				mtvDistance = overlapDist;
				seg = { p1, p2 };
			}
		}

		return nv::physics::CollisionData{ mtv, seg };
	};

	auto movingCollisionData = projectOntoEachPerpAxis(staticPoints);
	if (!movingCollisionData.has_value()) {
		return std::nullopt;
	}
	auto staticCollisionData = projectOntoEachPerpAxis(movingPoints);
	if (!staticCollisionData.has_value()) {
		return std::nullopt;
	}

	if (movingCollisionData->mtv.calcMagnitude() <= staticCollisionData->mtv.calcMagnitude()) {
    	return *movingCollisionData;
	} else {
		return *staticCollisionData;
	}
}
