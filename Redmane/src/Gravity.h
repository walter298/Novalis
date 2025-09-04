#pragma once

namespace rm {
    struct Gravity {
        static constexpr float STARTING_VELOCITY = 4.4f;

        float currVelocity = STARTING_VELOCITY;
        float acceleration = 1.12f;

        float getDownwardVelocity() noexcept {
            currVelocity *= acceleration;
            return currVelocity;
        }
        void reset() noexcept {
            currVelocity = STARTING_VELOCITY;
        }
    };
}