#pragma once

#include <optional>
#include <functional>
#include <string>

#include <nfd.hpp>

namespace nv {
	using FileExtensionFilters    = std::initializer_list<nfdfilteritem_t>;
	using FileOpenResult          = std::optional<std::string>;
	using MultipleFileOpensResult = std::optional<std::vector<std::string>>;
	using FileContentsGenerator   = std::function<std::string(const nfdchar_t*)>;

	FileOpenResult openFile(const FileExtensionFilters& filters);
	MultipleFileOpensResult openMultipleFiles(const FileExtensionFilters& filters);

	std::optional<std::string> openDirectory();
	std::optional<std::string> createNewFile(const FileExtensionFilters& filters);
	bool saveNewFile(const nv::FileExtensionFilters& filters, const FileContentsGenerator& stringGen);
	void saveToExistingFile(const std::string& filepath, const std::string& contents);

	const std::string& workingDirectory();

	//returns the path relative to the working directory
	const std::string& relativePath(std::string_view relativePath);

	std::optional<std::string> parseFileExtension(const std::string& fileName);
	const std::string& fileName(std::string_view filePath);

	std::string& convertFullToRegularPath(std::string& path);
	std::string convertFullToRegularPath(std::string_view path);
}