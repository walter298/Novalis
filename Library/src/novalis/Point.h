#pragma once

#include "detail/reflection/ClassIteration.h"

namespace nv {
	struct Point {
		float x = 0;
		float y = 0;

		void operator-=(Point other) noexcept {
			x -= other.x;
			y -= other.y;
		}
		void operator+=(Point other) noexcept {
			x += other.x;
			y += other.y;
		}
		void operator*=(Point other) noexcept{
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

		bool operator==(Point other) const noexcept {
			return x == other.x && y == other.y;
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
}