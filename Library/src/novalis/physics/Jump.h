#pragma once

#include <optional>

#include "Gravity.h"

namespace nv {
	namespace physics {
        class Jump {
        private:
            float m_maxHeight;
            float m_velocity;
            float m_distanceGoneUpward = 0.0f;
            float m_acceleration = 0.99f;

            void startMovingBackDown() {
                m_velocity *= -1.0f;
                m_acceleration = 1.01;
            }
        public:
            //please pass in the absolute values of maxHeight and velocity
            Jump(float maxHeight, float velocity) 
                : m_maxHeight{ maxHeight }, m_velocity{ -velocity } 
            {
            }

            std::optional<Point> move() noexcept {
                if (m_distanceGoneUpward > m_maxHeight) {
                    return std::nullopt;
                }

                m_velocity *= m_acceleration;
              
                //if we are moving up, check if we have surpassed the max height
                if (m_velocity < 0.0f) {
                    constexpr auto MIN_MAGNITUDE = 0.5f;

                    m_distanceGoneUpward += -m_velocity;
                    if (m_distanceGoneUpward > m_maxHeight || 
                        Point{ 0.0f, m_velocity }.calcMagnitude() < MIN_MAGNITUDE) 
                    {
                        startMovingBackDown();
                    } 
                }
                
                return Point{ 0.0f, m_velocity };
            }
        };
	}
}