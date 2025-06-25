#pragma once

#include "../LineSegment.h"
#include "../Polygon.h"
#include "../detail/reflection/TemplateDetection.h"
#include "Gravity.h"
#include "Intersection.h"

namespace nv {
	namespace physics {
		template<std::ranges::viewable_range PolygonStorage, typename Object, 
				 typename ObjectHitbox, Gravity Gravity = LinearGravity>
		class GravityManager {
		private:
			PolygonStorage m_polygonStorage;
			std::reference_wrapper<Object> m_object;
			std::reference_wrapper<ObjectHitbox> m_objectHitbox;
			Point m_hitboxCentroid;
			Gravity m_gravity;
			
			float m_surfaceX1 = 0.0f;
			float m_surfaceX2 = 0.0f;
			float m_surfaceSlope = 0.0f;
			bool m_falling = true;
			float m_dx = 0.0f;

			void move(Point d) noexcept {
				m_object.get().screenMove(d);
				m_object.get().worldMove(d);
				m_hitboxCentroid += d;
			}

			bool collideWithPolygons() noexcept {
				auto collide = [this](const auto& poly) {
					auto collisionData = calcCollisionData(unrefwrap(poly), m_objectHitbox.get());
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
		public:
			GravityManager(PolygonStorage& polygons, Object& object, ObjectHitbox& hitbox, Gravity g = Gravity{})
				: m_polygonStorage{ polygons }, m_object{ object }, m_objectHitbox{ hitbox },
				  m_hitboxCentroid{ hitbox.calcWorldCentroid() }, m_gravity{ g }
			{
			}
			
			void move(float dx) noexcept {
				m_dx += dx;
			}

			void operator()() {
				if (m_falling) {
					Point delta{ 0.0f, m_gravity.getDownwardVelocity() };
					move(delta);
				} 
				move(Point{ m_dx, 0.0f });
				if (collideWithPolygons()) { //todo: do not reset gravity if we hit a wall and not the ground
					m_falling = false;
					m_gravity.reset();
				} else {
					m_falling = true;
				}
				m_dx = 0.0f;
			}
		};
	}
}