#pragma once

#include <chrono>

namespace nv {
	class ID {
	private:
		int m_ID = 0;
	public:
		ID() noexcept;
		operator int() const noexcept;
	};
 
	struct SharedIDObject {
	private:
		ID m_ID;
	public:
		ID getID() const noexcept;
	};
}