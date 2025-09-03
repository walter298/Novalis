#pragma once

#include <optional>
#include <vector>
#include <boost/unordered/unordered_flat_map.hpp>
#include <novalis/Rect.h>

#include "File.h"

namespace nv {
	namespace editor {
		class ProjectFileManager;
		struct FileDialogSerializer;

		class FileDialogBase {
		private:
			DirectoryID m_currClickedDirectoryID = DirectoryID::None();
			DirectoryID m_currDirectoryID = DirectoryID::None();
			FileID m_currClickedFileID = FileID::None();
			std::vector<DirectoryID> m_directoryStack;

			void showDirectoryStack(const DirectoryMap& directories);
			void showFiles(const ProjectFileManager& pfm, const FileSet& displayedFiles, const FileMap& fileMap, File::Type filter);
			void showDirectories(const DirectorySet& displayedDirectories, const DirectoryMap& dirMap);
		protected:
			FileID getClickedFileID() const noexcept;

			void showFilesAndDirectories(const ProjectFileManager& pfm, const DirectoryMap& directories,
				const FileMap& files, File::Type filter);
			void showCreateButton(bool& clickedCreate, bool disabled);
			void showCancelButton(bool& cancelled);
		public:
			FileDialogBase();

			FileDialogBase(const FileDialogBase&) = delete;
			FileDialogBase(FileDialogBase&&) = default;
			FileDialogBase& operator=(FileDialogBase&&) = default;

			friend struct FileDialogSerializer;
		};

		class FileDialog : public FileDialogBase {
		public:
			using FileDialogBase::FileDialogBase;
			std::optional<FileID> show(const ProjectFileManager& pfm, const DirectoryMap& directories, 
				const FileMap& files, File::Type filter, bool& canceled);

			friend void from_json(const nlohmann::json& j, FileDialog& dialog);
			friend void to_json(nlohmann::json& j, const FileDialog& dialog);
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

			std::optional<FileSet> show(const ProjectFileManager& pfm, const DirectoryMap& directories,
				const FileMap& files, File::Type filter, bool& canceled);

			friend void from_json(const nlohmann::json& j, MultipleFileDialog& dialog);
			friend void to_json(nlohmann::json& j, const MultipleFileDialog& dialog);
		};
	}
}