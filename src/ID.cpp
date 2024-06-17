#include "ID.h"

nv::ID::ID() noexcept {
	static int IDCount = 0;
	m_ID = IDCount;
	IDCount++;
}

bool nv::ID::operator==(const ID& other) const noexcept {
	return (m_ID == other.m_ID);
}

const nv::ID& nv::IDObj::getID() const {
	return m_ID;
}