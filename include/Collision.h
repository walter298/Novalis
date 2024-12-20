#pragma once

#include <cmath>
#include <vector>

#include <boost/geometry.hpp>
#include <boost/geometry/geometries/point_xy.hpp>
#include <boost/geometry/geometries/polygon.hpp>

#include "data_util/BasicJsonSerialization.h"

namespace bg = boost::geometry;

namespace nv {
	using BGPoint = bg::model::point<int, 2, bg::cs::cartesian>;
	using BGPolygon = bg::model::polygon<BGPoint>;
	using BGTransform = bg::strategy::transform::translate_transformer<int, 2, 2>;
}

namespace boost {
	namespace geometry {
		namespace model {
			template<typename Geometry>
			void from_json(const nlohmann::json& j, Geometry& geo) {
				read_wkt(j.get<std::string>(), geo);
			}

			template<typename Point>
			void to_json(nlohmann::json& j, const polygon<Point>& point) {
				j = to_wkt(point);
			}
		}
	}
}

namespace nv {
	template<typename Geometry>
	void moveBGShape(Geometry& geometry, int dx, int dy) {
		for (const auto& point : geometry.outer()) {
			point.get<0>() += dx;
			point.get<1>() += dy;
		}
	}

	template<std::ranges::viewable_range Polygons>
	SDL_Point maxMovementWithoutIntersection(const BGPolygon& hitbox, int dx, int dy, const Polygons& polygons) 
		requires(std::same_as<std::ranges::range_value_t<Polygons>, BGPolygon>) 
	{
		auto movedHitbox = hitbox;
		moveBGShape(movedHitbox, dx, dy);

		auto touchedPolygons = polygons | std::views::filter([&](const auto& polygon) {
			return bg::intersects(movedHitbox, polygon);
		});
		//if we don't intersect with any polygons, we can simply move by dx and dy
		if (std::ranges::empty(touchedPolygons)) {
			return { dx, dy };
		}

		int maxXDist = std::abs(dx);
		int maxYDist = std::abs(dy);
		for (const auto& touchedPolygon : touchedPolygons) {
			bg::model::segment<BGPoint> closestPoints;
			bg::closest_points(hitbox, touchedPolygon, closestPoints);
			auto xDist = std::abs(closestPoints.second.get<0>() - closestPoints.first.get<0>());
			auto yDist = std::abs(closestPoints.second.get<1>() - closestPoints.first.get<1>());
			maxXDist = xDist < maxXDist ? xDist : maxXDist;
			maxYDist = yDist < maxYDist ? yDist : maxYDist;
		}
		
		return { std::copysign(maxXDist, dx), std::copysign(maxYDist, dy) };
	}
}