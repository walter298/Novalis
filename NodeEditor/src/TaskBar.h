#pragma once

#include "FileDropdown.h"
#include "LayerDropdown.h"
#include "ObjectDropdown.h"

namespace nv {
	namespace editor {
		class ProjectManager;
		class ErrorPopup;

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