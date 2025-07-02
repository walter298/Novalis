#pragma once

#include <novalis/detail/ScopeExit.h>

#include "NodeTabList.h"
#include "FileDropdown.h"
#include "LayerDropdown.h"
#include "ObjectDropdown.h"

namespace nv {
	namespace editor {
		class TaskBar {
		private:
			ObjectDropdown m_objectLoader;
			LayerDropdown m_layerCreator;
			FileDropdown m_fileDropdown;
		public:
			void show(SDL_Renderer* renderer, NodeTabList& tabs);
			bool isBusy() const noexcept;
		};
	}
}