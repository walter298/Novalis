#pragma once

#include "NodeManager.h"
#include "ProjectGraph.h"
#include "VirtualFilesystem.h"

namespace nv {
	namespace editor {
		class Project {
		private:
			NodeManager m_nodeManager;
			std::filesystem::path m_rootDirectory;
		public:
			VirtualFilesystem vfs;
			std::string name;

			Project() = default;
			Project(const std::filesystem::path& rootDirectory, const std::string& name) 
				: m_rootDirectory{ rootDirectory }, vfs{ rootDirectory }, name{ name }
			{
			}
			Project(const Project&) = delete;
			Project(Project&&)      = default;

			boost::optional<NodeEditor&> getCurrentTab();

			const std::filesystem::path& getRootDirectory() const noexcept {
				return m_rootDirectory;
			}

			void showTabs();
			void showFilesystem(ErrorPopup& errorPopup);

			friend void to_json(nlohmann::json& j, const Project& project);
			friend void from_json(const nlohmann::json& j, Project& project);
		};

		void to_json(nlohmann::json& j, const Project& project);
		void from_json(const nlohmann::json& j, Project& project);
	}
}