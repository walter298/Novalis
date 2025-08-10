#pragma once

#include <unordered_map>
#include <boost/optional.hpp>

#include "NodeEditor.h"
#include "Tab.h"

namespace nv {
	namespace editor {
		class VirtualFilesystem;
		class ErrorPopup;

		class TabManager {
		private:
			struct NodeData {
				NodeEditor editor;
				std::optional<BufferedNode> node;
				nlohmann::json nodeJson;
				bool saved = false;
			};
			std::unordered_map<FileID, NodeData> m_nodeTabs; //use node map since NodeEditors are gigantic
			std::unordered_map<FileID, nv::detail::TexturePtr> m_texTabs;
			Tab m_currTab;
			FileID m_switchedToInvalidTabID = FileID::None();
			NodeEditor* m_currNodeTab = nullptr;
			nv::detail::TexturePtr* m_currTexTab = nullptr;
			boost::unordered_flat_set<Tab> m_tabs;

			void switchTabs(const Tab& tab, VirtualFilesystem& vfs, ErrorPopup& errorPopup);
			void showCurrentTab(SDL_Renderer* renderer);
			void saveImpl(NodeData& nodeData, VirtualFilesystem& vfs);
			bool loadNode(VirtualFilesystem& vfs, FileID fileID, ErrorPopup& errorPopup);
			bool switchToNodeTab(VirtualFilesystem& vfs, FileID fileID, ErrorPopup& errorPopup);
			bool loadTexture(VirtualFilesystem& vfs, FileID fileID, ErrorPopup& errorPopup);
			bool switchToTextureTab(VirtualFilesystem& vfs, FileID fileID, ErrorPopup& errorPopup);
			void showImage();
			void showTabs(VirtualFilesystem& vfs, ErrorPopup& errorPopup);
			void updateCurrentTab(VirtualFilesystem& vfs, ErrorPopup& errorPopup);
		public:
			bool saveable(const VirtualFilesystem& vfs, ErrorPopup& errorPopup) const noexcept;
			void addNode(VirtualFilesystem& vfs, FileID id, ErrorPopup& errorPopup);
			void removeNode(FileID id);
			void save(VirtualFilesystem& vfs);
			void show(SDL_Renderer* renderer, VirtualFilesystem& vfs, ErrorPopup& errorPopup);

			boost::optional<NodeEditor&> getCurrentNodeTab();
		};
	}
}