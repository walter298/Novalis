#pragma once

#include <cmath>
#include <span>
#include <ranges>
#include <vector>
#include <boost/geometry.hpp>
#include <boost/geometry/geometries/point_xy.hpp>
#include <boost/geometry/geometries/polygon.hpp>
#include <boost/geometry/geometries/register/ring.hpp>
#include <boost/geometry/algorithms/transform.hpp>
#include <boost/geometry/strategies/transform/matrix_transformers.hpp>
#include <nlohmann/adl_serializer.hpp>
#include <SDL3/SDL_render.h>

#include "detail/memory/PointerUtil.h"
#include "detail/reflection/ClassIteration.h"
#include "physics/Pi.h"
#include "Point.h"

namespace nv {
	namespace detail {
		template<std::ranges::viewable_range PointStorage>
		struct Polygon {
			using Points = PointStorage;
			Points points;
			float currAngle = 0.0f;
			float currScale = 1.0f;
			Point rotationPoint;

			Polygon(PointStorage&& storage) : points{ std::move(storage) } {}

			Polygon(PointStorage&& storage, float angle, float scale) noexcept
				: points{ std::move(storage) }, currAngle{ angle }, currScale{ scale }
			{
			};

			Point getPos(size_t i = 0) const noexcept {
				return points[i];
			}
			Point setPos(Point newPos) noexcept {
				move(newPos - getPos());
				return newPos;
			}

			Point calcCentroid() const noexcept {
				Point ret;
				boost::geometry::centroid(points, ret);
				return ret;
			}

			void move(Point dMove) noexcept {
				for (auto& p : points) {
					p += dMove;
				}
			}

			void setRotation(float degrees, Point rotationPoint = {}) noexcept {
				currAngle = degrees;
				auto rad = degrees * (physics::pi / 180.0f);
				for (auto& [x, y] : points) {
					auto dx = x - rotationPoint.x;
					auto dy = y - rotationPoint.y;
					auto newX = (dx * std::cos(rad)) - (dy * std::sin(rad));
					auto newY = (dx * std::sin(rad)) + (dy * std::cos(rad));
					x = newX + rotationPoint.x;
					y = newY + rotationPoint.y;
				}
			}

			void scale(float newScale, Point refPoint = { 0.0f, 0.0f }) noexcept {
				if (newScale <= 0.0f) {
					return;
				}

				float scaleFactor = newScale / currScale;
				currScale = newScale;

				for (auto& [x, y] : points) {
					x = (x - refPoint.x) * scaleFactor + refPoint.x;
					y = (y - refPoint.y) * scaleFactor + refPoint.y;
				}
			}

			MAKE_INTROSPECTION(points, currAngle, currScale, rotationPoint)
		};

		using BufferedPolygonRep = Polygon<std::span<Point>>;
		using DynamicPolygonRep = Polygon<std::vector<Point>>;
	}
}

BOOST_GEOMETRY_REGISTER_RING(std::span<nv::Point>)
BOOST_GEOMETRY_REGISTER_RING(std::vector<nv::Point>)
BOOST_GEOMETRY_REGISTER_RING(std::vector<std::vector<nv::Point>>)

namespace boost::geometry::traits {
	template<typename PointStorage>
	struct point_type<nv::detail::Polygon<PointStorage>> {
		using type = nv::Point;
	};
	template<typename PointStorage>
	struct tag<nv::detail::Polygon<PointStorage>> {
		using type = polygon_tag;
	};
	template<typename PointStorage>
	struct ring_mutable_type<nv::detail::Polygon<PointStorage>> {
		using type = PointStorage&;
	};
	template<typename PointStorage>
	struct ring_const_type<nv::detail::Polygon<PointStorage>> {
		using type = const PointStorage&;
	};
	template<typename PointStorage>
	struct interior_mutable_type<nv::detail::Polygon<PointStorage>> {
		using type = std::vector<PointStorage>&;
	};
	template<typename PointStorage>
	struct interior_const_type<nv::detail::Polygon<PointStorage>> {
		using type = const std::vector<PointStorage>&;
	};

	template<typename PointStorage>
	struct exterior_ring<nv::detail::Polygon<PointStorage>> {
		static auto& get(nv::detail::Polygon<PointStorage>& poly) { 
			return poly.points; 
		}
		static auto& get(const nv::detail::Polygon<PointStorage>& poly) { 
			return poly.points; 
		}
	};

	template<typename PointStorage>
	struct interior_rings<nv::detail::Polygon<PointStorage>> {
		static auto& get(nv::detail::Polygon<PointStorage>& poly) {
			static std::vector<PointStorage> empty_interior_rings;
			return empty_interior_rings;  // Return an empty vector of interior rings.
		}
		static auto& get(const nv::detail::Polygon<PointStorage>& poly) {
			static const std::vector<PointStorage> empty_interior_rings;
			return empty_interior_rings;  // Return an empty vector of interior rings.
		}
	};
}

namespace nlohmann {
	struct PolygonSerializerBase;
}

namespace nv {
	namespace detail {
		/*void renderScreenPoints(SDL_Renderer* renderer, uint8_t opacity, const std::span<Point>& points, 
			SDL_Color color = { 255, 255, 255 }) 
		{
			SDL_Color originalDrawColor;
			SDL_GetRenderDrawColor(renderer, &originalDrawColor.r, &originalDrawColor.g, &originalDrawColor.b, &originalDrawColor.a);

			SDL_BlendMode originalBlendMode;
			SDL_GetRenderDrawBlendMode(renderer, &originalBlendMode);

			SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_ADD);
			SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, opacity);

			auto pointPairs = std::views::zip(points, points | std::views::drop(1));
			for (const auto& [p1, p2] : pointPairs) {
				SDL_RenderLine(renderer, p1.x, p1.y, p2.x, p2.y);
			}
			
			SDL_SetRenderDrawColor(renderer, originalDrawColor.r, originalDrawColor.g, originalDrawColor.b, originalDrawColor.a);
			SDL_SetRenderDrawBlendMode(renderer, originalBlendMode);
		}*/

		void renderScreenPoints(SDL_Renderer* renderer, uint8_t opacity, const std::span<const Point>& points,
			SDL_Color color = { 255, 255, 255 });

		struct PolygonConverter;

		template<typename PointStorage>
		struct PolygonNode {
		private:
			Polygon<PointStorage> m_ren;
			Polygon<PointStorage> m_world;
		public:
			PolygonNode(PointStorage&& screenPoints, float worldOffsetX, float worldOffsetY) 
			: m_ren{ std::move(screenPoints) }, m_world{ 
				[&, this] {
					auto worldPoints = m_ren.points;
					for (auto& [x, y] : worldPoints) {
						x += worldOffsetX;
						y += worldOffsetY;
					}
					return worldPoints;
				}() 
			}
			{
			}
			PolygonNode(Polygon<PointStorage>&& ren, Polygon<PointStorage>&& world)
				: m_ren{ std::move(ren) }, m_world{ std::move(world) }
			{
			}

			bool isValid() const noexcept {
				return boost::geometry::is_valid(m_ren);
			}

			uint8_t opacity = 0;

			Point getScreenPos(size_t i = 0) const noexcept {
				return m_ren.getPos(i);
			}
			Point getWorldPos(size_t i = 0) const noexcept {
				return m_world.getPos(i);
			}
			Point setScreenPos(Point p) noexcept {
				return m_ren.setPos(p);
			}
			Point setWorldPos(Point p) noexcept {
				return m_world.setPos(p);
			}

			Point calcScreenCentroid() const noexcept {
				return m_ren.calcCentroid();
			}
			Point calcWorldCentroid() const noexcept {
				return m_world.calcCentroid();
			}

			void screenMove(Point p) noexcept {
				m_ren.move(p);
			}

			void worldMove(Point p) noexcept {
				m_world.move(p);
			}

			void move(Point p) noexcept {
				screenMove(p);
				worldMove(p);
			}

			void screenScale(float scale) noexcept {
				m_ren.scale(scale);
			}
			void worldScale(float scale) noexcept {
				m_world.scale(scale);
			}
			float getScreenScale() const noexcept {
				return m_ren.currScale;
			}
			float getWorldScale() const noexcept {
				return m_world.currScale;
			}

			//todo: no way to draw a line with changed opacity
			void setOpacity(uint8_t opacityP) noexcept {
				opacity = opacityP;
			}

			bool containsScreenCoord(Point p) const noexcept {
				return boost::geometry::within(p, m_ren);
			}

			bool containsWorldCoord(Point p) const noexcept {
				return boost::geometry::within(p, m_world);
			}

			size_t getSize() const noexcept {
				return m_world.points.size();
			}

			bool isEmpty() const noexcept {
				return m_world.points.empty();
			}

			void render(SDL_Renderer* renderer) const noexcept {
				renderScreenPoints(renderer, opacity, std::span{ m_ren.points.data(), m_ren.points.size() });
			}

			const PointStorage& getScreenPoints() const noexcept {
				return m_ren.points;
			}
			const PointStorage& getWorldPoints() const noexcept {
				return m_world.points;
			}

			void resetWorld() noexcept {
				std::ranges::copy(m_ren.points, m_world.points.begin());
			}

			void setScreenRotation(float degrees) noexcept {
				m_ren.setRotation(degrees);
			}

			void setWorldRotation(float degrees, Point rotationPoint = {}) noexcept {
				m_world.setRotation(degrees);
			}

			void setRotation(double degrees, Point rotationPoint = {}) noexcept {
				m_ren.setRotation(degrees);
				m_world.setRotation(degrees);
			}

			friend struct nlohmann::PolygonSerializerBase;
			friend struct nlohmann::adl_serializer<PolygonNode<PointStorage>>;
			friend struct PolygonConverter;

			MAKE_INTROSPECTION(m_ren, m_world, opacity)
		};
	}

	using DynamicPolygon  = detail::PolygonNode<std::vector<Point>>;
	using BufferedPolygon = detail::PolygonNode<std::span<Point>>;

	namespace detail {
		struct PolygonConverter {
			static BufferedPolygon makeBufferedPolygon(const DynamicPolygon& polygon) {
				Polygon<std::span<Point>> ren{
					std::span{ const_cast<Point*>(polygon.m_ren.points.data()), polygon.m_ren.points.size() },
					polygon.m_ren.currAngle, polygon.m_ren.currScale
				};
				Polygon<std::span<Point>> world{
					std::span{ const_cast<Point*>(polygon.m_world.points.data()), polygon.m_world.points.size() },
					polygon.m_world.currAngle, polygon.m_world.currScale
				};

				return nv::BufferedPolygon{ std::move(ren), std::move(world) };
			}

			static void deepCopyBufferedPolygons(const std::byte* srcArena, std::byte* destArena,
				const nv::BufferedPolygon& srcPoly, nv::BufferedPolygon& destPoly)
			{
				//copy points
				auto copyPointSpan = [&](const std::span<Point>& src, std::span<Point>& dest) {
					Point* relativePtr{};
					matchOffset(srcArena, src.data(), destArena, relativePtr);
					dest = { relativePtr, src.size() };
					std::ranges::copy_n(src.data(), src.size(), dest.data());
				};
				copyPointSpan(srcPoly.m_ren.points, destPoly.m_ren.points);
				copyPointSpan(srcPoly.m_world.points, destPoly.m_world.points);

				//copy scale
				destPoly.m_ren.currScale = srcPoly.m_ren.currScale;
				destPoly.m_world.currScale = srcPoly.m_world.currScale;

				//copy angle
				destPoly.m_ren.currAngle = srcPoly.m_ren.currAngle;
				destPoly.m_world.currAngle = srcPoly.m_world.currAngle;

				//copy rotation point
				destPoly.m_ren.rotationPoint = srcPoly.m_ren.rotationPoint;
				destPoly.m_world.rotationPoint = srcPoly.m_world.rotationPoint;

				//copy opacity
				destPoly.opacity = srcPoly.opacity;
			}
		};
	}
}
