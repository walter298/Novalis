#pragma once

#include <cmath>
#include <span>
#include <vector>

#include <boost/geometry.hpp>
#include <boost/geometry/geometries/point_xy.hpp>
#include <boost/geometry/geometries/polygon.hpp>
#include <boost/geometry/geometries/register/point.hpp>
#include <boost/geometry/geometries/register/ring.hpp>

#include <SDL3/SDL_render.h>

#include "detail/memory/PointerUtil.h"
#include "detail/reflection/ClassIteration.h"
#include "Point.h"

namespace nv {
	namespace detail {
		//using BGPoint = bg::model::point<float, 2, bg::cs::cartesian>;
		using BGTransform = boost::geometry::strategy::transform::translate_transformer<float, 2, 2>;

		template<std::ranges::viewable_range PointStorage>
		struct Polygon {
			using Points = PointStorage;
			Points points;

			float currScale = 1.0f;

			template<typename... StorageArgs>
			Polygon(PointStorage&& storage) : points{ std::move(storage) }
			{
			};

			Point getPos(size_t i = 0) const noexcept {
				return points[i];
			}
			Point setPos(Point newPos) noexcept {
				move(newPos - getPos());
				return newPos;
			}
			void move(Point dMove) noexcept {
				for (auto& p : points) {
					p += dMove;
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

			MAKE_INTROSPECTION(points, currScale)
		};

		using BufferedPolygonRep = Polygon<std::span<Point>>;
		using DynamicPolygonRep = Polygon<std::vector<Point>>;
	}
}

BOOST_GEOMETRY_REGISTER_POINT_2D(nv::Point, float, boost::geometry::cs::cartesian, x, y)
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
		template<typename PointStorage>
		void renderScreenPoints(SDL_Renderer* renderer, uint8_t opacity, const PointStorage& points) {
			SDL_Color originalDrawColor;
			SDL_GetRenderDrawColor(renderer, &originalDrawColor.r, &originalDrawColor.g, &originalDrawColor.b, &originalDrawColor.a);

			SDL_BlendMode originalBlendMode;
			SDL_GetRenderDrawBlendMode(renderer, &originalBlendMode);

			SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_ADD);
			SDL_SetRenderDrawColor(renderer, 255, 255, 255, opacity);

			auto pointPairs = std::views::zip(points, points | std::views::drop(1));
			for (const auto& [p1, p2] : pointPairs) {
				SDL_RenderLine(renderer, p1.x, p1.y, p2.x, p2.y);
			}
			auto first = points.front();
			auto last = points.back();
			SDL_RenderLine(renderer, first.x, first.y, last.x, last.y);

			SDL_SetRenderDrawColor(renderer, originalDrawColor.r, originalDrawColor.g, originalDrawColor.b, originalDrawColor.a);
			SDL_SetRenderDrawBlendMode(renderer, originalBlendMode);
		}

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
				renderScreenPoints(renderer, opacity, m_ren.points);
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
				return nv::BufferedPolygon{
					std::span{ const_cast<Point*>(polygon.m_ren.points.data()), polygon.m_ren.points.size() },
					std::span{ const_cast<Point*>(polygon.m_world.points.data()), polygon.m_world.points.size() }
				};
			}

			static void deepCopyBufferedPolygons(const std::byte* srcArena, std::byte* destArena,
				const nv::BufferedPolygon& srcPoly, nv::BufferedPolygon& destPoly)
			{
				auto copyPointSpan = [&](const std::span<Point>& src, std::span<Point>& dest) {
					Point* relativePtr{};
					matchOffset(srcArena, src.data(), destArena, relativePtr);
					dest = { relativePtr, src.size() };
					std::ranges::copy_n(src.data(), src.size(), dest.data());
				};
				copyPointSpan(srcPoly.m_ren.points, destPoly.m_ren.points);
				copyPointSpan(srcPoly.m_world.points, destPoly.m_world.points);
				destPoly.m_ren.currScale = srcPoly.m_ren.currScale;
				destPoly.m_world.currScale = srcPoly.m_world.currScale;
				destPoly.opacity = srcPoly.opacity;
			}
		};
	}
}
