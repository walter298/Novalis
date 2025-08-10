#pragma once

#include "ErrorPopup.h"

namespace nv {
	namespace editor {
		class ProjectCreator {
		private:
			std::string m_projectNameInput;
			std::string m_directoryLocation = "Directory";
		public:
			bool create(bool& cancelled, ErrorPopup& errorPopup);
			const std::string& getCurrentDirectory();
			const std::string& getProjectNameInput();
		};
	}
}