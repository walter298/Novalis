#pragma once

#include <optional>
#include <functional>
#include <string>

#include <nfd.h>

namespace nv {
	using FileString     = std::basic_string<nfdchar_t>;
	using FileStringView = std::basic_string_view<nfdchar_t>;

	using FileExtensionFilters = std::vector<FileString>;
	using FileOpenResult = std::optional<FileString>;
	using MultipleFileOpensResult = std::optional<std::vector<FileString>>;

	FileOpenResult openFile(const FileExtensionFilters& filters);
	MultipleFileOpensResult openMultipleFiles(const FileExtensionFilters& filters);

	using FileContentsGenerator = std::function<std::string(FileStringView)>;
	bool saveNewFile(const nv::FileExtensionFilters& filters, const FileContentsGenerator& stringGen);

	const std::string& workingDirectory();

	//returns the path relative to the working directory
	const std::string& relativePath(std::string_view relativePath);

	std::optional<std::string> fileExtension(const std::string& fileName);
	std::string_view fileName(std::string_view filePath);

	std::string& convertFullToRegularPath(std::string& path);
	std::string convertFullToRegularPath(std::string_view path);
}