#include <print>

#include "NovalisRoot.h"

std::filesystem::path nv::editor::getNovalisRoot() {
	static constexpr const char* rootDirEnvVar = "NOVALIS_ROOT";
	static auto rootDirPath = std::getenv(rootDirEnvVar);
	if (!rootDirPath) {
		std::println(stderr, "Error: {} environment variable not set. Should be set to parent of NodeEditor and Library subdirectories.", rootDirEnvVar);
		std::exit(EXIT_FAILURE);
	}
	return rootDirPath;
}
