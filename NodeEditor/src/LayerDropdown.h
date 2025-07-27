#pragma once

#include <string>

#include "Project.h"

namespace nv {
	namespace editor {
		class LayerDropdown {
		private:
			std::string m_layerName;
			bool m_showingAddNewLayerPopup = false;

			void showLayerCreationWindow(NodeEditor& currTab);
		public:
			void show(Project& project);
		};
	}
}