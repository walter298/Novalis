#pragma once

#include <filesystem>
#include <optional>
#include <vector>
#include <boost/unordered/unordered_flat_map.hpp>
#include <novalis/detail/reflection/ClassIteration.h>

#include "File.h"

namespace nv {
	namespace editor {
		class ErrorPopup;

		class ProjectFileManager {
		private:
			std::filesystem::path m_rootDirPath;
			std::filesystem::path m_assetDirPath;
			std::filesystem::path m_versionHistoryDirPath;
			std::vector<std::filesystem::path> m_versionDirPaths;
			FileID::IntegerType m_fileIDOffset = 0;
			DirectoryID::IntegerType m_directoryIDOffset = 0;
			boost::unordered_flat_map<FileID, std::filesystem::path> m_sharedAssetPathMap;

			std::filesystem::path makeSharedAssetPath(FileID fileID, File::Type type) const;
			std::filesystem::path getNodeDirectoryPath(size_t projectIndex) const;
		public:
			ProjectFileManager() = default;
			ProjectFileManager(std::filesystem::path rootDirPath);
			ProjectFileManager(const ProjectFileManager&) = delete;
			ProjectFileManager(ProjectFileManager&&) = default;
			ProjectFileManager& operator=(ProjectFileManager&&) = default;
			
			FileID createImage(SDL_Surface* surface, File::Type imageType);
			std::optional<size_t> showVersionSelector(size_t currVersionIndex, bool& cancelled) const;
			void updateNodeFile(size_t projectIndex, FileID fileID, const std::string& data) const;
			void makeSharedAsset(FileID fileID, File::Type fileType, const std::string& data);
			void uploadSharedResource(FileID fileID, const std::filesystem::path& path);
			size_t addProjectVersion();
			size_t forkProjectVersion(size_t currProjectVersion);
			size_t getCurrentVersionCount() const noexcept;
			std::filesystem::path getFilesystemJSONPath(size_t projectIndex) const;
			std::filesystem::path getNodePath(size_t projectIndex, FileID fileID) const;
			const std::filesystem::path& getSharedAssetPath(FileID fileID) const;
			std::filesystem::path getGlobalProjectFilePath() const;

			MAKE_INTROSPECTION(m_rootDirPath, m_assetDirPath, m_versionHistoryDirPath,
				m_versionDirPaths, m_fileIDOffset, m_directoryIDOffset, m_sharedAssetPathMap)
		};
	}
}