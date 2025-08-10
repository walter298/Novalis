#pragma once

#include <boost/optional.hpp>

#include "Project.h"
#include "ProjectCreator.h"

namespace nv {
	namespace editor {
		class ProjectManager {
		private:
			ProjectCreator m_projectCreator;
			plf::hive<Project> m_projects;
			boost::unordered_flat_set<std::string> m_projectNames;
			Project* m_currProject = nullptr;
		public:
			void tryLoadProject(ErrorPopup& errorPopup);
			bool createProject(bool& cancelled, ErrorPopup& errorPopup);
			bool switchProject() noexcept; 
			boost::optional<Project&> getCurrentProject();
		};
	}
}