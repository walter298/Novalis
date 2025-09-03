#pragma once

#include "ErrorPopup.h"

namespace nv {
	namespace editor {
		class ProjectCreator {
		private:
			std::string m_projectNameInput;
			std::filesystem::path m_directoryLocation = "Directory";
		public:
			bool create(bool& cancelled, ErrorPopup& errorPopup);
			const std::filesystem::path& getCurrentDirectory();
			const std::string& getProjectNameInput();
		};
	}
}