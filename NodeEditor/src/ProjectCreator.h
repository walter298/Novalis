#pragma once

#include "ErrorPopup.h"
#include "ProjectManager.h"

namespace nv {
	namespace editor {
		class ProjectCreator {
		private:
			std::string m_projectNameInput;
			std::string m_directoryLocation = "Directory";
		public:
			bool create(ProjectManager& projectManager, ErrorPopup& errorPopup);
		};
	}
}