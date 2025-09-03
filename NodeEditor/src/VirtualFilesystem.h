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
		class ProjectFileManager;

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
			
			void showFile(FileID fileID, File& file, NameManager& parentNameManager);
			void deleteQueuedItems(TabManager& nodeManager, ErrorPopup& errorPopup);
			void uploadImage(DirectoryID parentDirID, Directory& parentDir, ProjectFileManager& pfm, ErrorPopup& errorPopup);
			FileID createNodeFile(DirectoryID parentDirID, Directory& parentDir);
			void createDirectory(DirectoryID parentDirID, Directory& parentDir);
			void renameDirectory(Directory& dir);
			void showFileRightClickMenu(FileID fileID, File& file, TabManager& nodeManager, ErrorPopup& errorPopup);
			void showDirectoryRightClickMenu(DirectoryID dirID, Directory& dir,
				TabManager& tabManager, ProjectFileManager& pfm, size_t projectIndex, bool& deleted,
				ErrorPopup& errorPopup) noexcept;
			void showImpl(DirectoryID dirID, TabManager& nodeManager, ProjectFileManager& pfm, 
				size_t projectIndex, ErrorPopup& errorPopup);
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
			
			//disable copying because there should only be one fs state at a time
			VirtualFilesystem(const VirtualFilesystem&) = delete;
			VirtualFilesystem& operator=(const VirtualFilesystem&) = delete;

			VirtualFilesystem(VirtualFilesystem&&) = default;
			VirtualFilesystem& operator=(VirtualFilesystem&&) = default;

			const FileSet& getDependantFiles(FileID fileID);
			FileID createNodeFile(const std::string& path);
			void show(TabManager& nodeManager, ProjectFileManager& pfm, size_t projectIndex, ErrorPopup& errorPopup); //returns a deleted file ID
			const std::string& getFilename(FileID fileID) const noexcept;
			std::optional<FileID> showFileDialog(const ProjectFileManager& pfm, File::Type filter, bool& cancelled);
			std::optional<FileSet> showMultipleFileDialog(const ProjectFileManager& pfm, File::Type filter, bool& cancelled);
			bool isTransitiveDependency(FileID parentID, FileID childID) const;
			bool createDependency(FileID fileID, FileID dependencyFileID);
			const File& getFile(FileID fileID) const;
			std::optional<Tab> getSelectedFile() const noexcept;
			
			MAKE_INTROSPECTION(m_files, m_directories, m_rootDirectoryID)
		};
	}
}