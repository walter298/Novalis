#include <filesystem>
#include <novalis/NodeFileVerification.h>
#include <novalis/detail/file/File.h>

namespace nv {
	namespace editor {
		namespace testing {
			inline void testNodeFileValidity(const std::initializer_list<std::string>& paths) {
				for (const auto& path : paths) {
					std::println("{} is {}", path, isNodeFileCorrect(path));
				}
			}
			inline void testNodeFileValidity(const std::filesystem::directory_iterator& dir) {
				for (const auto& path : dir) {
					if (path.is_directory()) {
						continue;
					}
					auto pathStr = path.path().string();
					auto isCorrect = isNodeFileCorrect(pathStr);
					std::println("{} is correct: {}", nv::fileName(pathStr), isCorrect);
				}
			}
		}
	}
}