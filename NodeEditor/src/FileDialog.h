#pragma once

#include <optional>
#include <vector>
#include <boost/unordered/unordered_flat_map.hpp>
#include <novalis/Rect.h>

#include "File.h"

namespace nv {
	namespace editor {
		class FileDialogBase {
		private:
			DirectoryID m_currClickedDirectoryID = DirectoryID::None();
			DirectoryID m_currDirectoryID;
			FileID m_currClickedFileID;
			std::vector<DirectoryID> m_directoryStack;

			void showDirectoryStack(const DirectoryMap& directories);
			void showFiles(const FileSet& displayedFiles, const FileMap& fileMap, File::Type filter);
			void showDirectories(const DirectorySet& displayedDirectories, const DirectoryMap& dirMap);
		protected:
			FileID getClickedFileID() const noexcept;

			void showFilesAndDirectories(const DirectoryMap& directories,
				const FileMap& files, File::Type filter);
			void showCreateButton(bool& clickedCreate, bool disabled);
			void showCancelButton(bool& cancelled);
		public:
			FileDialogBase(DirectoryID rootDirID);
		};

		class FileDialog : public FileDialogBase {
		public:
			using FileDialogBase::FileDialogBase;
			std::optional<FileID> show(const DirectoryMap& directories, const FileMap& files, File::Type filter, bool& canceled);
		};

		class MultipleFileDialog : public FileDialogBase {
		private:
			DirectoryID m_currDialogDirectoryID = DirectoryID::None();
			FileID m_currClickedFileDialogID = FileID::None();
			FileSet m_currChosenFileIDs;
			
			void showSelectedFiles(const FileMap& directories);
			void showAddButton();
		public:
			using FileDialogBase::FileDialogBase;

			std::optional<FileSet> show(const DirectoryMap& directories,
				const FileMap& files, File::Type filter, bool& canceled);
		};
	}
}