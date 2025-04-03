#pragma once

#include <boost/mpl/vector.hpp>
#include <boost/type_erasure/any.hpp>
#include <boost/type_erasure/member.hpp>
#include <SDL3/SDL_render.h>

#include "../Point.h"

namespace nv {
	namespace type_erased {
		namespace detail {
			BOOST_TYPE_ERASURE_MEMBER((has_render), render, 1)
			BOOST_TYPE_ERASURE_MEMBER((has_move), move, 1)
			BOOST_TYPE_ERASURE_MEMBER((has_screenMove), move, 1)
			BOOST_TYPE_ERASURE_MEMBER((has_worldMove), move, 1)
			BOOST_TYPE_ERASURE_MEMBER((has_getName), getName, 0)
			BOOST_TYPE_ERASURE_MEMBER((has_getScreenPos), getScreenPos, 0)
			BOOST_TYPE_ERASURE_MEMBER((has_getWorldPos), getWorldPos, 0)
			BOOST_TYPE_ERASURE_MEMBER((has_getScreenSize), getScreenSize, 0)
			BOOST_TYPE_ERASURE_MEMBER((has_getWorldSize), getWorldSize, 0)
			BOOST_TYPE_ERASURE_MEMBER((has_setScreenPos), setScreenPos, 1)
			BOOST_TYPE_ERASURE_MEMBER((has_setWorldPos), setWorldPos, 1)
			BOOST_TYPE_ERASURE_MEMBER((has_setScreenSize), setScreenSize, 1)
			BOOST_TYPE_ERASURE_MEMBER((has_setWorldSize), setWorldSize, 1)
		}

		namespace mpl = boost::mpl;
		namespace te = boost::type_erasure;

		using RenderableObject = te::any<
			mpl::vector<
				detail::has_render<void(SDL_Renderer*) const>,   
				detail::has_getName<std::string_view() const>,
				te::copy_constructible<>                   
			>
		>;

		using MoveableObject = te::any<
			mpl::vector<
				detail::has_render<void(SDL_Renderer*) const>,
				detail::has_move<void(Point)>,
				detail::has_screenMove<void(Point)>,
				detail::has_worldMove<void(Point)>,
				detail::has_getName<std::string_view() const>,
				detail::has_getScreenPos<Point() const>,
				detail::has_getWorldPos<Point() const>,
				te::copy_constructible<>
			>
		>;

		using ScaleableObject = te::any<
			mpl::vector<
				detail::has_render<void(SDL_Renderer*) const>,
				detail::has_getScreenSize<Point() const>,
				detail::has_getWorldSize<Point() const>,
				detail::has_setScreenSize<void(Point) const>,
				detail::has_setWorldSize<void(Point) const>,
				te::copy_constructible<>
			>
		>;
	}
}