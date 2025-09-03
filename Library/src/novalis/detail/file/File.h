#pragma once

#include <optional>
#include <filesystem>
#include <functional>

#include <nfd.hpp>

namespace nv {
	using FileExtensionFilters    = std::initializer_list<nfdfilteritem_t>;
	using FileOpenResult          = std::optional<std::filesystem::path>;
	using MultipleFileOpensResult = std::optional<std::vector<std::filesystem::path>>;
	using FileContentsGenerator   = std::function<std::filesystem::path(const nfdchar_t*)>;

	FileOpenResult openFile(const FileExtensionFilters& filters);
	MultipleFileOpensResult openMultipleFiles(const FileExtensionFilters& filters);

	std::optional<std::filesystem::path> openDirectory();
	std::optional<std::filesystem::path> createNewFile(const FileExtensionFilters& filters);
	bool saveNewFile(const nv::FileExtensionFilters& filters, const FileContentsGenerator& stringGen);
	void saveToExistingFile(const std::filesystem::path& filepath, const std::filesystem::path& contents);

	//returns reference to static local variable - no dangling!
	const std::filesystem::path& workingDirectory();
}