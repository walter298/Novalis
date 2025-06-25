#include <boost/mpl/vector.hpp>
#include <boost/type_erasure/any.hpp>
#include <boost/type_erasure/member.hpp>

#include "Point.h"

namespace nv {
    namespace detail {
		BOOST_TYPE_ERASURE_MEMBER((has_onCollision), onCollision, 1)
		BOOST_TYPE_ERASURE_MEMBER((has_move), move, 0)
    }
    using ObjectPath = boost::type_erasure::any<
        boost::mpl::vector<
            detail::has_onCollision<bool(float)>, //angle of the collided terrain is passed in
            detail::has_move<Point()>, //returns how much to move
            boost::type_erasure::copy_constructible<>   
        >
	>;

    struct MovePath {
        Point delta;

        MovePath(Point p) noexcept : delta{ delta } {}
        MovePath(float dx, float dy) noexcept : delta{ dx, dy } {}

        bool onCollision(float) const noexcept { return false; }
        Point move() const noexcept { return delta; }
    };
}

