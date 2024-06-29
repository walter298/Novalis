#include "ID.h"

nv::ID::ID() noexcept {
	static int IDCount = 0;
	m_ID = IDCount;
	IDCount++;
}

nv::ID::operator int() const noexcept {
	return m_ID;
}

nv::ID nv::SharedIDObject::getID() const noexcept {
	return m_ID;
}
