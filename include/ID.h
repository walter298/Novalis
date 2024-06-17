#pragma once

#include <chrono>

namespace nv {
	class ID {
	private:
		int m_ID = 0;
	public:
		ID() noexcept;
		bool operator==(const ID& other) const noexcept;
	};

	struct IDObj {
	private:
		ID m_ID;
	public:
		const ID& getID() const;
	};
}