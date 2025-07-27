#pragma once

#include <optional>
#include <novalis/detail/ScopeExit.h>

#include "NodeTabList.h"
#include "FileDropdown.h"
#include "LayerDropdown.h"
#include "ObjectDropdown.h"
#include "ProjectManager.h"

namespace nv {
	namespace editor {
		class TaskBar {
		private:
			ObjectDropdown m_objectLoader;
			LayerDropdown m_layerCreator;
			FileDropdown m_fileDropdown;
		public:
			void show(SDL_Renderer* renderer, ProjectManager& projectManager, ErrorPopup& errorPopup);
			bool isBusy() const noexcept;
		};
	}
}