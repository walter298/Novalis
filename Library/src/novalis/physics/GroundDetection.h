#include <optional>
#include <ranges>
#include <boost/geometry.hpp>

#include "Pi.h"
#include "Transform.h"
#include "../Polygon.h"

namespace nv {
	namespace physics {
		template<typename PointStorage, typename Object>
		class GroundDetection {
		private:
			using Polygon = detail::PolygonNode<PointStorage>;
			using LineSegment = boost::geometry::model::segment<Point>;

			std::reference_wrapper<const Polygon> m_ground;
			std::reference_wrapper<Object> m_object;
			std::reference_wrapper<Polygon> m_hitbox;
			std::reference_wrapper<Point> m_pointOfContact;
			LineSegment m_slope;
			bool m_standing = false;
		public:
			GroundDetection(Polygon& ground, Polygon& hitbox) noexcept
				: m_ground{ ground }, m_hitbox{ hitbox }
			{
			}

			//returns whether we are standing on the ground
			bool check(int dx) {
				
			}
		};

		template<typename PolygonStorage>
		std::optional<MovementTransform> adjustToGround(const detail::PolygonNode<PolygonStorage>& ground,
			Point pointOfContact, float slope) noexcept
		{
			assert(!ground.isEmpty());
			
			namespace bg = boost::geometry;
			if (!bg::within(pointOfContact, ground.getWorldPoints())) { //we're not touching the ground
				return std::nullopt;
			}

			using LineSegment = bg::model::segment<Point>;
			constexpr float 
			LineSegment objectSeg{ pointOfContact, pointOfContact * slope };

			auto groundLineSegments = std::views::zip(
				ground.getWorldPoints(),
				ground.getWorldPoints() | std::views::drop(1)
			);

			for (const auto& [p1, p2] : groundLineSegments) {
				LineSegment groundSeg{ p1, p2 };
			}

		}

		//template<typename Polygon>
		//class GroundDetection {
		//private:
		//	std::reference_wrapper<Polygon> m_ground;
		//	std::reference_wrapper<Polygon> m_objectHitbox;

		//	using LineSegment = bg::model::segment<Point>;
		//	LineSegment m_currSegment;
		//public:
		//	GroundDetection(Polygon& ground, Polygon& objectHitbox, const Point& contactPoint) noexcept
		//		: m_ground{ ground }, m_objectHitbox{ objectHitbox }
		//	{
		//		assert(!m_ground.get().isEmpty());
		//	}

		//	std::optional<float> adjustToGround() const noexcept {
		//		auto groundLineSegments = std::views::zip(
		//			m_ground.get().getWorldPoints(), m_ground.get().getWorldPoints() | std::views::drop(1)
		//		) | std::views::transform([](const auto& groundPointPair) {
		//			return LineSegment{ std::get<0>(groundPointPair), std::get<1>(groundPointPair) };
		//		});

		//		for (const auto& lineSegment : groundPointPairs) {
		//			if (bg::crosses(m_objectHitbox.get().getWorldPoints(), lineSegment)) {
		//				//calculate the point of intersection
		//				Point intersectionPoint;
		//				bg::intersection(m_objectHitbox.get().getWorldPoints(), lineSegment, intersectionPoint);
		//				auto dMove = intersectionPoint - m_objectHitbox.get().getPos();
		//				
		//				//calculate the angle of the line segment
		//				auto azimuthRad = boost::geometry::azimuth(lineSegment.get<0>(), lineSegment.get<1>());
		//				auto angle = azimuthRad * (180.0f / pi);

		//				return angle;
		//			}
		//		}
		//		return std::nullopt; //no ground detected
		//	}
		//};
	}
}