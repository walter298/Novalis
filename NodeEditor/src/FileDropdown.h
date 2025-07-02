#include "NodeTabList.h"

namespace nv {
	namespace editor {
		class FileDropdown {
		private:
			std::string m_nodeNameInput;
			bool m_showingNodeCreationPopupWindow = false;

			void showNodeCreationPopupWindow(NodeTabList& tabs);
		public:
			void show(NodeTabList& tabs);
		};
	}
}