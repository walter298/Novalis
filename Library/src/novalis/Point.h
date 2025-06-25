#pragma once

#include <boost/geometry/core/coordinate_system.hpp>
#include <boost/geometry/geometries/register/point.hpp>

#include <SDL3/SDL_rect.h>

#include "detail/reflection/ClassIteration.h"

namespace nv {
	struct Point {
		float x = 0.0f;
		float y = 0.0f;

		void operator-=(Point other) noexcept {
			x -= other.x;
			y -= other.y;
		}
		void operator+=(Point other) noexcept {
			x += other.x;
			y += other.y;
		}
		void operator*=(Point other) noexcept {
			x *= other.x;
			y *= other.y;
		}
		void operator/=(Point other) noexcept {
			x /= other.x;
			y /= other.y;
		}
		void operator-=(float a) noexcept {
			x -= a;
			y -= a;
		}
		void operator+=(float a) noexcept {
			x += a;
			y += a;
		}
		void operator*=(float a) noexcept {
			x *= a;
			y *= a;
		}
		void operator/=(float a) noexcept {
			x /= a;
			y /= a;
		}

		Point operator-() const noexcept {
			return { -x, -y };
		}

		bool operator==(Point other) const noexcept {
			return x == other.x && y == other.y;
		}

		float calcMagnitude() const noexcept {
			return std::sqrt(x * x + y * y);
		}

		Point normalize() const noexcept {
			auto magnitude = calcMagnitude();
			assert(magnitude != 0.0f);
			return { x / magnitude, y / magnitude };
		}

		operator SDL_FPoint() const noexcept {
			return { x, y };
		}

		MAKE_INTROSPECTION(x, y)
	};

	static_assert(detail::MemberIterable<Point>);

	inline Point operator-(Point a, Point b) noexcept {
		return { a.x - b.x, a.y - b.y };
	}
	inline Point operator+(Point a, Point b) noexcept {
		return { a.x + b.x, a.y + b.y };
	}
	inline Point operator*(Point a, Point b) noexcept {
		return { a.x * b.x, a.y * b.y };
	}
	inline Point operator/(Point a, Point b) noexcept {
		return { a.x / b.x, a.y / b.y };
	}
	inline Point operator*(Point a, float b) noexcept {
		return { a.x * b, a.y * b };
	}
	inline Point operator/(Point a, float b) noexcept {
		return { a.x / b, a.y / b };
	}
	inline Point operator*(float b, Point a) noexcept {
		return { a.x * b, a.y * b };
	}
}

BOOST_GEOMETRY_REGISTER_POINT_2D(nv::Point, float, boost::geometry::cs::cartesian, x, y)