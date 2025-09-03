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
			LayerDropdown m_layerCreator; //todo: make functional
		public:
			void show(SDL_Renderer* renderer, ProjectManager& projectManager, ErrorPopup& errorPopup);
			bool isBusy() const noexcept;
		};
	}
}