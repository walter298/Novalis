#pragma once

#include <concepts>
#include <memory>
#include <ranges>
#include <vector>
#include <nlohmann/json.hpp>
#include <novalis/ID.h>
#include <novalis/detail/reflection/ClassIteration.h>
#include <novalis/detail/serialization/AutoSerialization.h>

#include "ErrorPopup.h"
#include "FileDialog.h"
#include "ImageType.h"
#include "Tab.h"

namespace nv {
	namespace editor {
		class TabManager;

		class VirtualFilesystem {
		private:
			FileMap m_files;
			DirectoryMap m_directories;
			DirectoryID m_rootDirectoryID; //IMPORTANT: this must be declared before file dialog members
			FileDialog m_fileDialog;
			MultipleFileDialog m_multiFileDialog;
			DirectoryID m_renamingDirectoryID = DirectoryID::None();
			FileID m_selectedFileID = FileID::None();
			FileID m_renamingFileID = FileID::None();
			FileID m_draggedFileID = FileID::None();
			DirectoryID m_deletedDirectoryID = DirectoryID::None();
			FileID m_deletedFileID = FileID::None();
			std::filesystem::path m_projectRootDirectoryPath;

			void showFile(FileID fileID, File& file, NameManager& parentNameManager);
			void deleteQueuedItems(TabManager& nodeManager, ErrorPopup& errorPopup);
			std::filesystem::path getInternalDirectoryPath() const;
			std::filesystem::path getAssetDirectoryPath() const;
			std::filesystem::path getNodeDirectoryPath() const;
			void uploadImage(DirectoryID parentDirID, Directory& parentDir, ErrorPopup& errorPopup);
			FileID createNodeFile(DirectoryID parentDirID, Directory& parentDir);
			void createDirectory(DirectoryID parentDirID, Directory& parentDir);
			void renameDirectory(Directory& dir);
			void showFileRightClickMenu(FileID fileID, File& file, TabManager& nodeManager, ErrorPopup& errorPopup);
			void showDirectoryRightClickMenu(DirectoryID dirID, Directory& dir, TabManager& nodeManager, 
				bool& deleted, ErrorPopup& errorPopup) noexcept;
			void showImpl(DirectoryID dirID, TabManager& nodeManager, ErrorPopup& errorPopup);
			void setCurrentlyRenamedDirectory(DirectoryID dirID);
			void setCurrentlyRenamedFile(FileID fileID);
			void dragFile(FileID draggedFileID, File& draggedFile);
			void dropFile(DirectoryID dirID);
			bool isDependencyForFilesOutsideDirectory(DirectoryID dirID, const File& file, ErrorPopup& errorPopup) const;
			bool isDirectoryDeletable(DirectoryID topLevelDirID, DirectoryID subdirectoryID, ErrorPopup& errorPopup) const;
			bool tryDelete(FileID fileID, File& file, TabManager& nodeManager, ErrorPopup& errorPopup);
			void deleteDirectoryImpl(DirectoryID dirID, Directory& dir, TabManager& nodeManager);
			bool tryDeleteDirectory(DirectoryID dirID, Directory& dir, TabManager& nodeManager, ErrorPopup& errorPopup);
			bool isSubdirectory(DirectoryID childDirID, DirectoryID parentDirID) const;
		public:
			VirtualFilesystem();
			VirtualFilesystem(std::filesystem::path projectRootDirectoryPath);

			const FileSet& getDependantFiles(FileID fileID);
			FileID createNodeFile(const std::string& path);
			void show(TabManager& nodeManager, ErrorPopup& errorPopup); //returns a deleted file ID
			const std::string& getFilename(FileID fileID) const noexcept;
			const std::filesystem::path& getFilePath(FileID fileID) const noexcept;
			std::optional<FileID> showFileDialog(File::Type filter, bool& cancelled);
			std::optional<FileSet> showMultipleFileDialog(File::Type filter, bool& cancelled);
			nv::detail::TexturePtr getTexture(FileID fileID);
			FileID saveImage(SDL_Surface* surface, ImageType imageType);
			void dumpFileContents(FileID fileID, const std::string& text);
			void createDependency(FileID fileID, FileID dependencyFileID);

			std::optional<Tab> getSelectedFile() const noexcept;
			std::filesystem::path getNodePath(FileID fileID) const;

			MAKE_INTROSPECTION(m_rootDirectoryID, m_files, m_directories, m_projectRootDirectoryPath)
		};
	}
}