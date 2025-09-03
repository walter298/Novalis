#pragma once

#include <unordered_map>
#include <boost/optional.hpp>

#include "NodeEditor.h"
#include "Tab.h"

namespace nv {
	namespace editor {
		class ProjectFileManager;
		class ErrorPopup;
		class VirtualFilesystem;

		class TabManager {
		public:
			struct NodeTab {
				NodeEditor editor;
				std::optional<BufferedNode> node;
				nlohmann::json nodeJson;
				bool saved = false;
			};
		private:
			std::unordered_map<FileID, NodeTab> m_nodeTabs; //use node map since NodeEditors are gigantic
			std::unordered_map<FileID, nv::detail::TexturePtr> m_texTabs;
			Tab m_currTab;
			FileID m_switchedToInvalidTabID = FileID::None();
			NodeEditor* m_currNodeTab = nullptr;
			nv::detail::TexturePtr* m_currTexTab = nullptr;
			boost::unordered_flat_set<Tab> m_tabs;

			void switchTabs(const Tab& tab, ProjectFileManager& pfm, size_t projectIndex, ErrorPopup& errorPopup);
			void showCurrentTab(SDL_Renderer* renderer);
			bool loadNode(const ProjectFileManager& vfs, size_t projectIndex, FileID fileID, ErrorPopup& errorPopup);
			bool switchToNodeTab(ProjectFileManager& vfs, size_t projectIndex, FileID fileID, ErrorPopup& errorPopup);
			bool loadTexture(ProjectFileManager& vfs, FileID fileID, ErrorPopup& errorPopup);
			bool switchToTextureTab(ProjectFileManager& vfs, FileID fileID, ErrorPopup& errorPopup);
			void showImage();
			void showTabs(VirtualFilesystem& vfs, ProjectFileManager& pfm,size_t projectIndex, ErrorPopup& errorPopup);
			void updateCurrentTab(VirtualFilesystem& vfs, ProjectFileManager& pfm, 
				size_t projectIndex, ErrorPopup& errorPopup);
		public:
			bool saveable(const VirtualFilesystem& vfs, ErrorPopup& errorPopup) const noexcept;
			void addNode(ProjectFileManager& vfs, size_t projectIndex, FileID id, ErrorPopup& errorPopup);
			boost::optional<NodeTab&> getNodeTab(const ProjectFileManager& pfm, size_t projectIndex,
				FileID id, ErrorPopup& errorPopup);
			void removeNode(FileID id);
			void show(SDL_Renderer* renderer, VirtualFilesystem& vfs, ProjectFileManager& pfm, size_t projectIndex, ErrorPopup& errorPopup);
			void clear() noexcept;

			boost::optional<NodeEditor&> getCurrentNodeTab();

			auto begin(this auto&& self) {
				return self.m_nodeTabs.begin();
			}
			auto end(this auto&& self) {
				return self.m_nodeTabs.end();
			}
		};
	}
}