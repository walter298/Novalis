#pragma once

#include <optional>
#include <span>

#include "../LineSegment.h"
#include "../Polygon.h"

namespace nv {
	namespace physics {
		bool intersects(const std::span<Point>& a, const std::span<Point>& b) noexcept;

		template<typename PointStorage1, typename PointStorage2>
		bool intersects(const detail::PolygonNode<PointStorage1>& a, const detail::PolygonNode<PointStorage2>& b) noexcept
		{
			const auto& aPoints = a.getWorldPoints();
			const auto& bPoints = b.getWorldPoints();

			return intersects(std::span{ aPoints.data(), aPoints.size() }, std::span{ bPoints.data(), bPoints.size() });
		}

		struct CollisionData {
			Point mtv;
			LineSegment collidedStaticSegment;
		};

		std::optional<CollisionData> calcCollisionData(const std::span<Point>& staticPoints, const std::span<Point>& movingPoints) noexcept;
		
		template<typename PointStorage1, typename PointStorage2>
		std::optional<CollisionData> calcCollisionData(const detail::PolygonNode<PointStorage1>& staticPoly, 
			const detail::PolygonNode<PointStorage2>& movingPoly) noexcept
		{
			const auto& staticPoints = staticPoly.getWorldPoints();
			const auto& movingPoints = movingPoly.getWorldPoints();

			return calcCollisionData(
				std::span{ staticPoints.data(), staticPoints.size() }, 
				std::span{ movingPoints.data(), movingPoints.size() }
			);
		}
	}
}