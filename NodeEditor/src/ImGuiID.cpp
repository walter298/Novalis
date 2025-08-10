#include <filesystem>
#include <fstream>
#include <nlohmann/json.hpp>

#include "ImGuiID.h"
#include "NovalisRoot.h"

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

namespace {
	std::filesystem::path getIDFilePath() {
		static auto path = nv::editor::getNovalisRoot() / "IDS.json";
		return path;
	}
	
	constexpr const char* PERMANENT_ID_OFFSET_KEY = "Permanent_ID_Offset";
}

void nv::editor::saveIDs() {
	std::ofstream file{ getIDFilePath() };
	assert(file.is_open());
	nlohmann::json j;
	j[PERMANENT_ID_OFFSET_KEY] = permIDCount;
	file << j.dump(2);
}

void nv::editor::initIDs() {
	auto filePath = getIDFilePath();
	if (!std::filesystem::exists(filePath)) {
		saveIDs();
	}
	std::ifstream file{ filePath };
	assert(file.is_open());
	auto j = nlohmann::json::parse(file);
	permIDCount = j[PERMANENT_ID_OFFSET_KEY].get<int>();
}
