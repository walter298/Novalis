#include "NodeTabList.h"
#include "ProjectCreator.h"
#include "ProjectManager.h"

namespace nv {
	namespace editor {
		class FileDropdown {
		private:
			bool m_creatingProject = false;
			ProjectCreator m_projectCreator;
		public:
			void show(ProjectManager& projectManager, ErrorPopup& errorPopup);
		};
	}
}