#include "ImGuiID.h"

namespace {
	int tempIDCount = 0;
	constexpr int MAX_TEMP_ID_COUNT = 0x15000;
	int permIDCount = MAX_TEMP_ID_COUNT + 1;
}

int nv::editor::getPermanentImGuiID() {
	permIDCount++;
	return permIDCount;
}

int nv::editor::getTemporaryImGuiID() noexcept {
	tempIDCount++;
	return tempIDCount;
}

void nv::editor::resetTemporaryImGuiIDs() noexcept {
	tempIDCount = 0;
}
