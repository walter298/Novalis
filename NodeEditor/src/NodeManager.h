#pragma once

#include <unordered_map>
#include <boost/optional.hpp>

#include "NodeEditor.h"

namespace nv {
	namespace editor {
		class VirtualFilesystem;

		class NodeManager {
		private:
			std::unordered_map<FileID, NodeEditor> m_nodes; //use nodes since NodeEditors are gigantic
			NodeEditor* m_currTab = nullptr;
		public:
			void addNode(FileID id);
			void removeNode(FileID id);
			void showTabs(VirtualFilesystem& fs);

			boost::optional<NodeEditor&> getCurrentTab();
		};
	}
}