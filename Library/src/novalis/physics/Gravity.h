#include <concepts>

#include "../Point.h"

namespace nv {
	namespace physics {
		template<typename T>
		concept Gravity = std::is_default_constructible_v<T> && requires(T gravity) {
			gravity.reset();
			{ gravity.getDownwardVelocity() } -> std::same_as<float>;
		};

		class LinearGravity {
		private:
			float m_velocity;
		public:
			explicit LinearGravity(float velocity = 10.0f) : m_velocity{ velocity }
			{
			}
			float getDownwardVelocity() const noexcept {
				return m_velocity;
			}
			void reset() {} //no-op
		};
	}
}