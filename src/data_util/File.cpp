#include "File.h"

#include <algorithm>
#include <cassert>
#include <fstream>
#include <filesystem>
#include <numeric>
#include <ranges>

#include "Algorithms.h"
#include "DataStructures.h"

namespace {
	struct Path {
		nfdchar_t* path = nullptr;
		~Path() {
			free(path);
		}
	};

	auto makeFilterList(const nv::FileExtensionFilters& filters) {
		constexpr auto delim = ";";
		auto combinedFilters = std::accumulate(filters.begin(), filters.end(), nv::FileString{}, [&](const auto& a, const auto& b) {
			return a + delim + b;
		});
		return combinedFilters;
	}
}
nv::FileOpenResult nv::openFile(const nv::FileExtensionFilters& filters) {
	Path outPath;
	auto combinedFilters = makeFilterList(filters);
	auto result = NFD_OpenDialog(combinedFilters.c_str(), nullptr, &outPath.path);
	if (result == NFD_OKAY) {
		return FileString{ outPath.path };
	} else {
		return std::nullopt;
	}
}

nv::MultipleFileOpensResult nv::openMultipleFiles(const nv::FileExtensionFilters& filters) {
	nfdpathset_t outPaths;

	auto combinedFilters = makeFilterList(filters);
	auto res = NFD_OpenDialogMultiple(nullptr, nullptr, &outPaths);
	if (res == NFD_CANCEL) {
		return std::nullopt;
	}

	nv::ScopeExit scopeExit{ [&] { NFD_PathSet_Free(&outPaths); } };
	
	std::vector<FileString> ret;
	auto pathC = NFD_PathSet_GetCount(&outPaths);
	ret.reserve(pathC);
	for (size_t i = 0; i < pathC; i++) {
		ret.push_back(NFD_PathSet_GetPath(&outPaths, i));
	}
	return ret;
}

bool nv::saveNewFile(const nv::FileExtensionFilters& filters, const nv::FileContentsGenerator& contentsGen) {
	auto combinedFilters = makeFilterList(filters);
	Path outPath;

	auto res = NFD_SaveDialog(combinedFilters.c_str(), nullptr, &outPath.path);
	
	if (res == NFD_CANCEL || res == NFD_ERROR) {
		return false;
	}

	std::ofstream file{ outPath.path };
	file << contentsGen(outPath.path);
	file.close();

	return true;
}

std::string& nv::convertFullToRegularPath(std::string& path) {
	auto relativePathSize = relativePath("").size();
	path.erase(path.begin(), path.begin() + relativePathSize);
	return path;
}

std::string nv::convertFullToRegularPath(std::string_view path) {
	std::string pathStr = path.data();
	convertFullToRegularPath(pathStr);
	return pathStr;
}

const std::string& nv::workingDirectory() { //should be called by nv::Instance constructor
	static auto path = [] {
		auto path = std::filesystem::current_path().string() + "/";
		std::ranges::replace(path, '\\', '/');
		return path;
	}();
	return path;
}

/*Have thread local string to prevent dangling pointers when relativePath is assigned to string_view*/
const std::string& nv::relativePath(std::string_view relativePath) {
	thread_local std::string global;
	global = workingDirectory() + relativePath.data();
	return global;
}

std::optional<std::string> nv::fileExtension(const std::string& fileName) {
	using namespace std::literals;

	auto dotPos = std::ranges::find(fileName, '.');
	if (dotPos == fileName.end()) {
		return std::nullopt;
	}
	return std::accumulate(dotPos, fileName.end(), ""s);
}

std::string_view nv::fileName(std::string_view filePath) {
	static std::string nonDanglingFilename;
	nonDanglingFilename = filePath.data();

	auto slashIdx = nonDanglingFilename.find_last_of('\\');
	if (slashIdx == std::string::npos) {
		slashIdx = nonDanglingFilename.find_last_of('/');
		assert(slashIdx != std::string::npos);
	}
	auto dotIdx = nonDanglingFilename.find_last_of('.');
	assert(dotIdx != std::string::npos);
	nonDanglingFilename = filePath.substr(slashIdx + 1, dotIdx - slashIdx - 1);

	return nonDanglingFilename;
}
