#include "ProjectManager.h"

void nv::editor::ProjectManager::addProject(const std::string& path, const std::string& name) {
	m_currProject = &(*m_projects.emplace(path, name));
	int x = 0;
}

boost::optional<nv::editor::Project&> nv::editor::ProjectManager::getCurrentProject() {
	if (m_currProject) {
		return *m_currProject;
	} else {
		return boost::none;
	}
}
