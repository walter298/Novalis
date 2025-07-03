#pragma once

#include "../LineSegment.h"
#include "../Polygon.h"
#include "../detail/reflection/TemplateDetection.h"
#include "Gravity.h"
#include "Intersection.h"
#include "../Instance.h"

namespace nv {
	namespace physics {
		template<std::ranges::viewable_range PolygonStorage, typename Object, 
				 typename ObjectHitbox, Gravity Gravity = LinearGravity>
		class GravityManager {
		private:
			PolygonStorage m_polygonStorage;
			Object m_object;
			Point m_lowestHitboxPoint;
			Point m_lowestRenderPoint;
			std::reference_wrapper<ObjectHitbox> m_objectHitbox;
			Gravity m_gravity;
			
			float m_surfaceX1 = 0.0f;
			float m_surfaceX2 = 0.0f;
			float m_surfaceSlope = 0.0f;
			bool m_falling = true;
			float m_dx = 0.0f;

			void move(Point delta) noexcept {
				nv::detail::unrefwrap(m_object).move(delta);
				m_lowestHitboxPoint += delta;
				m_lowestRenderPoint += delta;
			}

			bool collideWithPolygons() noexcept {
				auto collide = [this](const auto& poly) {
					using namespace nv::detail; //for unrefwrap

					auto collisionData = calcCollisionData(unrefwrap(poly), unrefwrap(m_objectHitbox));
					if (collisionData) {
						auto [mtv, collidedSeg] = *collisionData;
						m_surfaceSlope = boost::geometry::azimuth(collidedSeg.first, collidedSeg.second);
						m_surfaceX1 = collidedSeg.first.x;
						m_surfaceX2 = collidedSeg.second.x;
						move(mtv);
						return true;
					}
					return false;
				};
				for (const auto& poly : unrefwrap(m_polygonStorage)) {
					if (collide(poly)) {
						return true;
					}
				}
				return false;
			}

			float calcHighestAlignedSegmentDistance() {
				auto closestDistance = std::numeric_limits<float>::max();

				auto calcDistance = [&, this](LineSegment seg) {
					if (seg.first.x == seg.second.x) { //skip degenerate edges
						return;
					}
					if (seg.second.x < seg.first.x) {
						std::swap(seg.first, seg.second);
					}

					//make sure that our point is aligned with the line segment horizontally
					if (!(m_lowestHitboxPoint.x >= seg.first.x && m_lowestHitboxPoint.x <= seg.second.x)) {
						return;
					}
					auto distAlongSegment = m_lowestHitboxPoint.x - seg.first.x;
					auto slope = (seg.second.y - seg.first.y) / (seg.second.x - seg.first.x);
					auto alignedY = seg.first.y + (slope * distAlongSegment);
					auto dist = alignedY - m_lowestHitboxPoint.y;

					//don't count line segments that are above the object
					if (dist > 0.0f && dist < closestDistance) {
						closestDistance = dist;
					}
				};

				auto findMinImpl = [&, this](const auto& worldPoints) {
					auto pointPairs = std::views::zip(worldPoints, worldPoints | std::views::drop(1));
					for (const auto& [p1, p2] : pointPairs) {
						LineSegment seg{ p1, p2 };
						calcDistance(seg);
					}
					calcDistance({ worldPoints[0], worldPoints.back() });
				};

				for (const auto& polyRef : m_polygonStorage) {
					auto& poly = nv::detail::unrefwrap(polyRef);
					const auto& worldPoints = poly.getWorldPoints();
					findMinImpl(worldPoints);
				}

				return closestDistance;
			}

			void landOnGround() {
				m_falling = false;
				m_gravity.reset();
			}

			void fall() {
				auto minDistance = calcHighestAlignedSegmentDistance();
				auto velocity = m_gravity.getDownwardVelocity();
				assert(velocity >= 0.0f);

				if (velocity > minDistance) { //prevent tunneling through the ground
					move({ 0.0f, minDistance });
					landOnGround();
				} else {
					move({ 0.0f, velocity });
				}
			}
		public:
			template<typename ObjectFR>
			GravityManager(PolygonStorage& polygons, ObjectFR&& object, ObjectHitbox& hitbox, Gravity g = Gravity{})
				: m_polygonStorage{ polygons }, m_object{ std::forward<ObjectFR>(object) },
				m_objectHitbox{ hitbox }, m_gravity{ g }
			{
				const auto& worldPoints = hitbox.getWorldPoints();
				m_lowestHitboxPoint = std::ranges::max(worldPoints, {}, [](const auto& point) {
					return point.y; 
				});
			}
			
			void move(float dx) noexcept {
				m_dx += dx;
			}

			auto& getPolygons(this auto&& self) noexcept {
				return nv::detail::unrefwrap(self.m_polygonStorage);
			}

			auto& getObject(this auto&& self) noexcept {
				return nv::detail::unrefwrap(self.m_object);
			}

			auto& getHitbox(this auto&& self) noexcept {
				return nv::detail::unrefwrap(self.m_objectHitbox);
			}

			void operator()() {
				if (m_falling) {
					fall();
				} 

				move(Point{ m_dx, 0.0f });

				if (collideWithPolygons()) { //todo: do not reset gravity if we hit a wall and not the ground
					landOnGround();
				} else {
					m_falling = true;
				}
				m_dx = 0.0f;
			}
		};
	}
}