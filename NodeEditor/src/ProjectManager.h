#pragma once

#include <boost/optional.hpp>

#include "Project.h"

namespace nv {
	namespace editor {
		class ProjectManager {
		private:
			plf::hive<Project> m_projects;
			Project* m_currProject = nullptr;
		public:
			void addProject(const std::string& path, const std::string& name);
			boost::optional<Project&> getCurrentProject();
		};
	}
}