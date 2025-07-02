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
			Object m_object;
			std::reference_wrapper<ObjectHitbox> m_objectHitbox;
			Gravity m_gravity;
			
			float m_surfaceX1 = 0.0f;
			float m_surfaceX2 = 0.0f;
			float m_surfaceSlope = 0.0f;
			bool m_falling = true;
			float m_dx = 0.0f;

			void move(Point delta) noexcept {
				nv::detail::unrefwrap(m_object).move(delta);
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
		public:
			template<typename ObjectFR>
			GravityManager(PolygonStorage& polygons, ObjectFR&& object, ObjectHitbox& hitbox, Gravity g = Gravity{})
				: m_polygonStorage{ polygons }, m_object{ std::forward<ObjectFR>(object) }, 
				m_objectHitbox{ hitbox }, m_gravity{ g }
			{
			}
			
			void move(float dx) noexcept {
				m_dx += dx;
			}

			auto& getPolygons(this auto&& self) noexcept {
				return unrefwrap(self.m_polygonStorage);
			}

			auto& getObject(this auto&& self) noexcept {
				return unrefwrap(self.m_object);
			}

			auto& getHitbox(this auto&& self) noexcept {
				return unrefwrap(self.m_objectHitbox);
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