#include <novalis/detail/serialization/BufferedNodeSerialization.h>

#include "ProjectFileManager.h"
#include "TabManager.h"
#include "VirtualFilesystem.h"

//for static methods
using namespace nv;
using namespace editor;

void nv::editor::TabManager::switchTabs(const Tab& tab, ProjectFileManager& pfm, size_t projectIndex, 
	ErrorPopup& errorPopup) 
{
	if (tab.type & File::Type::Image) {
		if (switchToTextureTab(pfm, tab.id, errorPopup)) {
			m_currTab = tab;
		} else {
			m_switchedToInvalidTabID = tab.id;
		}
	} else if (tab.type & File::Type::Node) {
		if (switchToNodeTab(pfm, projectIndex, tab.id, errorPopup)) {
			m_currTab = tab;
		} else {
			m_switchedToInvalidTabID = tab.id;
		}
	} 
}

void nv::editor::TabManager::showCurrentTab(SDL_Renderer* renderer) {
	if (m_currTab.id == FileID::None()) {
		return;
	}
	if (m_currTab.type == File::Type::Node) {
		m_currNodeTab->show(renderer);
	} else if (m_currTab.type & File::Type::Image) {
		showImage();
	}
}

bool nv::editor::TabManager::loadNode(const ProjectFileManager& pfm, size_t projectIndex, FileID fileID,
	ErrorPopup& errorPopup)
{
	try {
		auto path = pfm.getNodePath(projectIndex, fileID);
		if (!std::filesystem::exists(path)) {
			auto& [id, tab] = *m_nodeTabs.emplace(
				std::piecewise_construct, std::forward_as_tuple(fileID), std::forward_as_tuple(fileID)
			).first;
			m_currNodeTab = &tab.editor;
			return true;
		}
		std::ifstream file{ path };
		auto nodeJson = nlohmann::json::parse(file);

		auto nodeEditor = NodeEditor::load(nodeJson, fileID, errorPopup);
		if (!nodeEditor) {
			return false;
		}
		auto instance = getGlobalInstance();
		auto node = instance->registry.loadBufferedNode(path, nodeJson);
		auto& [id, tab] = *m_nodeTabs.emplace(
			std::piecewise_construct, std::forward_as_tuple(fileID), std::forward_as_tuple(fileID)
		).first;
		tab.node = std::move(node);
		tab.editor = std::move(*nodeEditor);
		m_currNodeTab = &tab.editor;
		return true;
	} catch (const std::exception& e) {
		errorPopup.add(std::format("Error: {}", e.what()));
	}
	return false;
}

bool nv::editor::TabManager::switchToNodeTab(ProjectFileManager& pfm, size_t projectIndex, FileID fileID, 
	ErrorPopup& errorPopup) 
{
	auto currNodeIt = m_nodeTabs.find(fileID);
	if (currNodeIt != m_nodeTabs.end()) {
		m_currNodeTab = &currNodeIt->second.editor;
		return true;
	} else {
		return loadNode(pfm, projectIndex, fileID, errorPopup);
	}
}

bool nv::editor::TabManager::loadTexture(ProjectFileManager& pfm, FileID fileID, 
	ErrorPopup& errorPopup) 
{
	auto instance = getGlobalInstance();
	try {
		auto path = pfm.getSharedAssetPath(fileID);
		auto tex = instance->registry.loadTexture(instance->getRenderer(), path);
		auto [it, tab] = m_texTabs.emplace(fileID, std::move(tex));
		m_currTexTab = &it->second;
		return true;
	} catch (const std::exception& e) {
		errorPopup.add(std::format("Error: {}", e.what()));
	}
	return false;
}

bool nv::editor::TabManager::switchToTextureTab(ProjectFileManager& pfm, FileID fileID, 
	ErrorPopup& errorPopup)
{
	auto currNodeIt = m_texTabs.find(fileID);
	if (currNodeIt != m_texTabs.end()) {
		m_currTexTab = &currNodeIt->second;
		return true;
	} else {
		return loadTexture(pfm, fileID, errorPopup);
	}
}

bool nv::editor::TabManager::saveable(const VirtualFilesystem& vfs, ErrorPopup& errorPopup) const noexcept {
	for (const auto& [id, tab] : m_nodeTabs) {
		if (tab.editor.hasNoLayers()) {
			const auto& tabName = vfs.getFilename(tab.editor.getID());
			errorPopup.add(std::format("Error: cannot save project because {} has no layers", tabName));
			return false;
		}
	}
	return true; 
}

void nv::editor::TabManager::addNode(ProjectFileManager& pfm, size_t projectIndex, FileID id, ErrorPopup& errorPopup) {
	switchToNodeTab(pfm, projectIndex, id, errorPopup);
}

boost::optional<nv::editor::TabManager::NodeTab&> nv::editor::TabManager::getNodeTab(const ProjectFileManager& pfm, size_t projectIndex,
	FileID id, ErrorPopup& errorPopup)
{
	auto nodeIt = m_nodeTabs.find(id);
	if (nodeIt == m_nodeTabs.end()) {
		if (!loadNode(pfm, projectIndex, id, errorPopup)) {
			return boost::none;
		}
		return m_nodeTabs.at(id);
	} 
	return nodeIt->second;
}

void nv::editor::TabManager::removeNode(FileID id) {
	if (m_currNodeTab && m_currNodeTab->getID() == id) {
		m_currNodeTab = nullptr;
	}
	m_nodeTabs.erase(id);
}

void nv::editor::TabManager::showTabs(VirtualFilesystem& vfs, ProjectFileManager& pfm, 
	size_t projectIndex, ErrorPopup& errorPopup) 
{
	if (ImGui::BeginTabBar("Tabs")) {
		for (auto& [id, type] : m_tabs) {
			ImGui::PushID(getTemporaryImGuiID());
			
			if (ImGui::BeginTabItem(vfs.getFilename(id).c_str())) {
				switch (type) {
				case File::Type::Node:
					switchToNodeTab(pfm, projectIndex, id, errorPopup);
					break;
				case File::Type::Image:
					switchToTextureTab(pfm, id, errorPopup);
					break;
				}
				ImGui::EndTabItem();
			}
			ImGui::PopID();
		}
		ImGui::EndTabBar();
	}
}

void nv::editor::TabManager::updateCurrentTab(VirtualFilesystem& vfs, ProjectFileManager& pfm, 
	size_t projectIndex, ErrorPopup& errorPopup) 
{
	auto selectedTab = vfs.getSelectedFile();
	if (!selectedTab) {
		m_currTexTab = nullptr;
		m_currTexTab = nullptr;
		return;
	}
	if (selectedTab->id != m_currTab.id && selectedTab->id != m_switchedToInvalidTabID) {
		switchTabs(*selectedTab, pfm, projectIndex, errorPopup);
	}
}

void nv::editor::TabManager::showImage() {
	ImVec2 imageSize{ 
		static_cast<float>(m_currTexTab->tex->w), 
		static_cast<float>(m_currTexTab->tex->h)
	};
	clampImVec2(imageSize, getTabWindowSize());
	ImGui::Image(reinterpret_cast<ImTextureID>(m_currTexTab->tex), imageSize);
}

void nv::editor::TabManager::show(SDL_Renderer* renderer, VirtualFilesystem& vfs, 
	ProjectFileManager& pfm, size_t projectIndex, ErrorPopup& errorPopup) 
{
	auto tabWindowPos = getTabWindowPos();
	auto tabWindowSize = getTabWindowSize();
	ImGui::SetNextWindowPos(tabWindowPos);
	ImGui::SetNextWindowSize(tabWindowSize);

	ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4{ 0.0f, 0.0f, 0.0f, 0.f });

	ImGui::Begin(TAB_WINDOW_NAME, nullptr, DEFAULT_WINDOW_FLAGS);

	showTabs(vfs, pfm, projectIndex, errorPopup);
	updateCurrentTab(vfs, pfm, projectIndex, errorPopup);
	showCurrentTab(renderer);

	ImGui::PopStyleColor();

	ImGui::End();
}

void nv::editor::TabManager::clear() noexcept {
	m_tabs.clear();
	m_currNodeTab = nullptr;
	m_currTexTab = nullptr;
	m_switchedToInvalidTabID = FileID::None();
	m_currTab = { FileID::None(), File::Type::None };
}

boost::optional<nv::editor::NodeEditor&> nv::editor::TabManager::getCurrentNodeTab() {
	if (m_currNodeTab) {
		return *m_currNodeTab;
	} else {
		return boost::none;
	}
}