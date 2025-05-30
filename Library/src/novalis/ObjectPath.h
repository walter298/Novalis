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

    class JumpPath {
    private:
        float m_distanceGoneUpward = 0.0f;
        float m_maxHeight = 0.0f;
        float m_startingVelocity = 0.0f;
        float m_currVelocity = 0.0f;
        float m_acceleration = 0.0f;
        float m_maxGroundSteepness = 0.0f;
        float m_direction = 1.0;

        void switchDirection() {
            m_acceleration = 1.0f / m_acceleration; //reverse acceleration
            m_direction *= -1.0f;
            m_distanceGoneUpward = 0.0f;
        }
    public:
        JumpPath(float maxHeight, float startingVelocity, float acceleration, float maxGroundSteepness) 
            : m_maxHeight{maxHeight }, m_startingVelocity{ startingVelocity }, 
            m_acceleration{ acceleration }, m_maxGroundSteepness{ maxGroundSteepness }
        {
            assert(m_maxGroundSteepness < 90.0f);
        }

        bool onCollision(float terrainAngle) noexcept {
            switchDirection();

            if (terrainAngle < m_maxGroundSteepness) { //if we've hit the ground
                m_currVelocity = m_startingVelocity;
                return true;
            } else { //if we've hit the roof
                return false;
            }
        }

        Point move() noexcept {
            if (m_distanceGoneUpward > m_maxHeight) {
                switchDirection();
            }
            m_currVelocity *= m_acceleration;
            return { 0.0f, m_currVelocity };
        }
    };
}

