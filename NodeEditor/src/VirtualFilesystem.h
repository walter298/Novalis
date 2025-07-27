#pragma once

#include <concepts>
#include <memory>
#include <ranges>
#include <vector>
#include <nlohmann/json.hpp>
#include <novalis/ID.h>
#include <novalis/detail/serialization/AutoSerialization.h>

#include "ErrorPopup.h"
#include "FileDialog.h"
#include "ImageType.h"
#include "NodeManager.h"
#include "ProjectGraph.h"

namespace nv {
	namespace editor {
		class VirtualFilesystem {
		private:
			static inline nv::detail::TexturePtr openFolderTex;
			static inline nv::detail::TexturePtr closedFolderTex;
			static inline nv::detail::TexturePtr nodeFileIconTex;
			static inline nv::detail::TexturePtr pngFileIconTex;
			static inline nv::detail::TexturePtr avifFileIconTex;
			static inline nv::detail::TexturePtr jpgFileIconTex;
			static inline nv::detail::TexturePtr bmpFileIconTex;

			static ImTextureID getFileIcon(std::string fileExtension);

			using ErrorMessage = std::string;

			FileMap m_files;
			DirectoryMap m_directories;
			DirectoryID m_rootDirectoryID; //IMPORTANT: this must be declared before file dialog members
			FileDialog m_fileDialog;
			MultipleFileDialog m_multiFileDialog;
			DirectoryID m_renamingDirectoryID = DirectoryID::None();
			FileID m_renamingFileID = FileID::None();
			FileID m_draggedFileID = FileID::None();
			DirectoryID m_deletedDirectoryID = DirectoryID::None();
			FileID m_deletedFileID = FileID::None();
			std::filesystem::path m_projectRootDirectoryPath;

			void deleteQueuedItems(NodeManager& nodeManager, ErrorPopup& errorPopup);
			std::filesystem::path getAssetDirectoryPath() const;
			std::filesystem::path getNodeDirectoryPath() const;
			void uploadImage(DirectoryID parentDirID, Directory& parentDir, ErrorPopup& errorPopup);
			FileID createNodeFile(DirectoryID parentDirID, Directory& parentDir);
			void createDirectory(DirectoryID parentDirID);
			void createNodeFile(NodeManager& nodeManager);
			void showFileRightClickMenu(FileID fileID, File& file, NodeManager& nodeManager, bool& deleted, ErrorPopup& errorPopup);
			void showDirectoryRightClickMenu(DirectoryID dirID, Directory& dir, NodeManager& nodeManager, 
				bool& deleted, ErrorPopup& errorPopup) noexcept;
			void showImpl(DirectoryID dirID, NodeManager& nodeManager, ErrorPopup& errorPopup);
			void setCurrentlyRenamedDirectory(DirectoryID dirID);
			void setCurrentlyRenamedFile(FileID fileID);
			void renameDirectory(Directory& dir);
			void showFile(NameManager& dirNameManager, FileID fileID, File& file);
			void dragFile(FileID draggedFileID, File& draggedFile, DirectoryID dirID);
			void dropFile(DirectoryID dirID);
			bool isDependencyForFilesOutsideDirectory(DirectoryID dirID, const File& file, ErrorPopup& errorPopup) const;
			bool isDirectoryDeletable(DirectoryID topLevelDirID, DirectoryID subdirectoryID, ErrorPopup& errorPopup) const;
			bool tryDelete(FileID fileID, File& file, NodeManager& nodeManager, ErrorPopup& errorPopup);
			void deleteDirectoryImpl(DirectoryID dirID, Directory& dir, NodeManager& nodeManager);
			bool tryDeleteDirectory(DirectoryID dirID, Directory& dir, NodeManager& nodeManager, ErrorPopup& errorPopup);
			bool isSubdirectory(DirectoryID childDirID, DirectoryID parentDirID) const;
		public:
			static void loadFolderTextures(SDL_Renderer* renderer) noexcept;
			static void destroyFolderTextures();

			VirtualFilesystem();
			VirtualFilesystem(std::filesystem::path projectRootDirectoryPath);

			FileID createNodeFile(const std::string& path);
			void show(NodeManager& nodeManager, ErrorPopup& errorPopup);
			const std::string& getFilename(FileID fileID) const noexcept;
			std::optional<FileID> showFileDialog(File::Type filter, bool& cancelled);
			std::optional<FileSet> showMultipleFileDialog(File::Type filter, bool& cancelled);

			nv::detail::TexturePtr getTexture(FileID fileID);
			FileID saveImage(SDL_Surface* surface, ImageType imageType);
			void dumpFileContents(FileID fileID, const std::string& text);
			void createDependency(FileID fileID, FileID dependencyFileID);
		private:
			void saveDirectory(const Directory& directory, nlohmann::json& j) const;
			void loadFromJsonImpl(const json& dirJson, DirectoryID dirID);
		public:
			void dumpJson(json& j) const;
			void loadFromJson(const json& root);
		};

		void to_json(json& j, const VirtualFilesystem& vfs);
		void from_json(const json& j, VirtualFilesystem& vfs);
	}
}