#pragma once

#include <optional>
#include <boost/unordered/unordered_flat_map.hpp>
#include <novalis/Rect.h>

#include "File.h"

namespace nv {
	namespace editor {
		class FileDialogBase {
		protected:
			DirectoryID m_currDialogDirectoryID = DirectoryID::None();
			FileID m_currClickedFileDialogID = FileID::None();

			virtual void resetState() = 0;
			virtual void handleFile(FileID fileID) = 0;
			void showFilesAndDirectories(const DirectoryMap& directories,
				const FileMap& files, File::Type filter);
			void showCreateButton(bool& clickedCreate);
			void showCancelButton(bool& cancelled);
		public:
			FileDialogBase(DirectoryID rootDirID) : m_currDialogDirectoryID{ rootDirID }
			{
			}
		};

		class FileDialog : public FileDialogBase {
		private:
			void handleFile(FileID fileID) final override;
			void resetState() final override;
		public:
			using FileDialogBase::FileDialogBase;
			std::optional<FileID> show(const DirectoryMap& directories, const FileMap& files, File::Type filter, bool& canceled);
		};

		class MultipleFileDialog : public FileDialogBase {
		private:
			DirectoryID m_currDialogDirectoryID = DirectoryID::None();
			FileID m_currClickedFileDialogID = FileID::None();
			FileSet m_currChosenFileIDs;
			FileSet m_selectedFileIDs;
			Rect m_selectionRect;
			bool m_setSelection = false;

			void updateSelectionRect() noexcept;
			void resetState() final override;
			void handleFile(FileID fileID) final override;
			void showAddButton();
			void showCurrentlyChosenFileIDs(const FileMap& files);
		public:
			using FileDialogBase::FileDialogBase;

			std::optional<FileSet> show(const DirectoryMap& directories,
				const FileMap& files, File::Type filter, bool& canceled);
		};
	}
}