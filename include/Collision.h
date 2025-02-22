#pragma once

#include <cmath>
#include <vector>

#include <boost/geometry.hpp>
#include <boost/geometry/geometries/point_xy.hpp>
#include <boost/geometry/geometries/polygon.hpp>

#include <SDL3/SDL_render.h>

#include "data_util/BasicJsonSerialization.h"

namespace bg = boost::geometry;

namespace nv {
	using BGPoint = bg::model::point<float, 2, bg::cs::cartesian>;
	using BGPolygon = bg::model::polygon<BGPoint>;
	using BGTransform = bg::strategy::transform::translate_transformer<float, 2, 2>;

	SDL_FPoint toSDLFPoint(BGPoint p) noexcept;
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
	struct Polygon {
		BGPolygon rep;
		float currScale = 1.0f;

		inline SDL_FPoint getScreenPos() const {
			return { rep.outer()[0].get<0>(), rep.outer()[0].get<1>() };
		}
		void move(SDL_FPoint p) noexcept;

		inline void scale(float newScale, SDL_FPoint refPoint = { 0.0f, 0.0f }) noexcept {
			if (newScale <= 0.0f) {
				return;
			}

			float scaleFactor = newScale / currScale;
			currScale = newScale;

			for (auto& point : rep.outer()) {
				auto x = bg::get<0>(point);
				auto y = bg::get<1>(point);

				bg::set<0>(point, (x - refPoint.x) * scaleFactor + refPoint.x);
				bg::set<1>(point, (y - refPoint.y) * scaleFactor + refPoint.y);
			}
		}

		bool containsCoord(SDL_FPoint p) const noexcept;
		void save(json& j) const;
	};

	
	struct RenderPolygon {
	private:
		Polygon m_world;
		Polygon m_ren;
	public:
		uint8_t opacity = 0;

		inline SDL_FPoint getScreenPos() const noexcept {
			return { m_ren.rep.outer()[0].get<0>(), m_ren.rep.outer()[0].get<1>() };
		}
		inline SDL_FPoint getWorldPos() const noexcept {
			return { m_world.rep.outer()[0].get<0>(), m_world.rep.outer()[0].get<1>() };
		}

		inline void screenMove(SDL_FPoint p) noexcept {
			m_ren.move(p);
		}
		inline void worldMove(SDL_FPoint p) noexcept {
			m_world.move(p);
		}

		inline void screenScale(float factor, SDL_FPoint refPoint = { 0.0f, 0.0f }) noexcept {
			m_ren.scale(factor, refPoint);
		}

		inline void render(SDL_Renderer* renderer) const noexcept {
			SDL_SetRenderDrawColor(renderer, 0, 255, 0, opacity);
			auto points = std::views::zip(m_ren.rep.outer(), m_ren.rep.outer() | std::views::drop(1));
			for (const auto& [p1, p2] : points) {
				SDL_RenderLine(renderer, p1.get<0>(), p1.get<1>(), p2.get<0>(), p2.get<1>());
			}
			auto first = m_ren.rep.outer().front();
			auto last = m_ren.rep.outer().back();
			SDL_RenderLine(renderer, first.get<0>(), first.get<1>(), last.get<0>(), last.get<1>());
		}

		//todo: no way to draw a line with changed opacity
		inline void setOpacity(uint8_t opacityP) noexcept {
			opacity = opacityP;
		}

		inline void clear() {
			m_ren.rep.clear();
			m_world.rep.clear();
		}

		inline void move(SDL_FPoint p) {
			m_ren.move(p);
			m_world.move(p);
		}

		inline void add(BGPoint p) {
			m_ren.rep.outer().push_back(p);
			m_world.rep.outer().push_back(std::move(p));
		}

		inline void add(SDL_FPoint p) {
			m_ren.rep.outer().emplace_back(p.x, p.y);
			m_world.rep.outer().emplace_back(p.x, p.y);
		}

		inline BGPoint getWorldCoord(size_t i) const noexcept {
			return m_world.rep.outer()[i];
		}
		inline BGPoint getScreenCoord(size_t i) const noexcept {
			return m_ren.rep.outer()[i];
		}

		inline size_t getSize() const noexcept {
			return m_ren.rep.outer().size();
		}

		inline bool isEmpty() const noexcept {
			return m_ren.rep.outer().empty();
		}
	};
}