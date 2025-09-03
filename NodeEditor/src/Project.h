#pragma once

#include "TabManager.h"
#include "VirtualFilesystem.h"
#include "ProjectFileManager.h"

namespace nv {
	namespace editor {
		class Project {
		private:
			size_t m_currProjectVersion = 0;
			FileID::IntegerType m_fileIDOffset = 0;
			DirectoryID::IntegerType m_directoryIDOffset = 0;
			std::string m_name;

			void loadFilesystemVersion(size_t index);	
			void writeFilesystemJson();
			void writeGlobalProjectJson();
			void saveImpl(TabManager::NodeTab& tab, ErrorPopup& errorPopup);
		public:
			ProjectFileManager pfm;
			TabManager tabManager;
			VirtualFilesystem vfs;
			
			Project() = default;
			Project(std::filesystem::path rootDirPath, std::string name);
			Project(const Project&) = delete;
			Project(Project&&)      = default;

			FileID::IntegerType getFileIDOffset() const noexcept;
			DirectoryID::IntegerType getDirectoryIDOffset() const noexcept;
			bool switchVersion(bool& cancelled, ErrorPopup& errorPopup);
			void show(SDL_Renderer* renderer, ErrorPopup& errorPopup);
			bool save(ErrorPopup& errorPopup);
			size_t getCurrentVersion() const noexcept;
			const std::string& getName() const noexcept;	

			friend void from_json(const nlohmann::json& j, Project& project);
		};
	}
}