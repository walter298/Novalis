#pragma once

namespace nv {
	namespace editor {
		class ProjectCreator;
		class ProjectManager;
		class ErrorPopup;

		class FileDropdown {
		private:
			enum State {
				None,
				CreatingProject,
				SwitchingProject
			} m_state = None;
		public:
			void show(ProjectManager& projectManager, ErrorPopup& errorPopup);
		};
	}
}