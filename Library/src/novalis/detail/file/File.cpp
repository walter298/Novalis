#include "File.h"

#include <algorithm>
#include <cassert>
#include <fstream>
#include <filesystem>
#include <numeric>
#include <ranges>

std::optional<std::filesystem::path> nv::openDirectory() {
	NFD::UniquePath outPath;
	auto dirResult = NFD::PickFolder(outPath);
	if (dirResult == NFD_OKAY) {
		return std::filesystem::path{ outPath.get() };
	}
	return std::nullopt;
}

nv::FileOpenResult nv::openFile(const nv::FileExtensionFilters& filters) {
	NFD::UniquePath outPath;
	
	auto result = NFD::OpenDialog(outPath, filters.begin(), static_cast<nfdfiltersize_t>(filters.size()));
	if (result == NFD_OKAY) {
		return std::filesystem::path{ outPath.get() };
	} else {
		return std::nullopt;
	}
}

nv::MultipleFileOpensResult nv::openMultipleFiles(const nv::FileExtensionFilters& filters) {
	NFD::UniquePathSet outPaths = nullptr;

	auto res = NFD::OpenDialogMultiple(outPaths, filters.begin(), static_cast<nfdfiltersize_t>(filters.size()));
	if (res != NFD_OKAY) {
		return std::nullopt;
	}

	nfdpathsetsize_t pathC = 0;
	if (NFD::PathSet::Count(outPaths, pathC) != NFD_OKAY) {
		return std::nullopt;
	}

	std::vector<std::filesystem::path> strPaths;
	strPaths.reserve(pathC);
	for (nfdpathsetsize_t i = 0; i < pathC; i++) {
		NFD::UniquePathSetPath currPath;
		NFD::PathSet::GetPath(outPaths, i, currPath);
		strPaths.emplace_back(currPath.get());
	}
	return strPaths;
}

std::optional<std::filesystem::path> nv::createNewFile(const FileExtensionFilters& filters) {
	NFD::UniquePath path;
	if (NFD::SaveDialog(path, filters.begin(), static_cast<nfdfiltersize_t>(filters.size())) != NFD_OKAY) {
		return std::nullopt;
	}
	return std::filesystem::path{ path.get() };
}

bool nv::saveNewFile(const nv::FileExtensionFilters& filters, const nv::FileContentsGenerator& contentsGen) {
	NFD::UniquePath path;
	if (NFD::SaveDialog(path, filters.begin(), static_cast<nfdfiltersize_t>(filters.size())) != NFD_OKAY) {
		return false;
	}

	std::ofstream file{ path.get() };
	file << contentsGen(path.get());
	file.close();

	return true;
}

void nv::saveToExistingFile(const std::filesystem::path& filepath, const std::filesystem::path& contents) {
	std::filesystem::remove(filepath);
	std::ofstream file{ filepath };
	file << contents;
	file.close();
}

const std::filesystem::path& nv::workingDirectory() { //should be called by nv::Instance constructor
	static auto path = [] {
		auto path = std::filesystem::current_path();
		return path;
	}();
	return path;
}