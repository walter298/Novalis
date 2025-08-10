#include "TabManager.h"
#include "TabManager.h"
#include "TabManager.h"
#include "TabManager.h"
#include "TabManager.h"
#include <novalis/detail/serialization/BufferedNodeSerialization.h>

#include "TabManager.h"
#include "VirtualFilesystem.h"

//for static methods
using namespace nv;
using namespace editor;

void nv::editor::TabManager::switchTabs(const Tab& tab, VirtualFilesystem& vfs, ErrorPopup& errorPopup) {
	switch (tab.type) {
	case File::Type::Node:
		if (switchToNodeTab(vfs, tab.id, errorPopup)) {
			m_currTab = tab;
		} else {
			m_switchedToInvalidTabID = tab.id;
		}
		break;
	case File::Type::Image:
		if (switchToTextureTab(vfs, tab.id, errorPopup)) {
			m_currTab = tab;
		} else {
			m_switchedToInvalidTabID = tab.id;
		}
		break;
	}
}

void nv::editor::TabManager::showCurrentTab(SDL_Renderer* renderer) {
	if (m_currTab.id == FileID::None()) {
		return;
	}
	switch (m_currTab.type) {
	case File::Type::Node:
		m_currNodeTab->show(renderer);
		break;
	case File::Type::Image:
		showImage();
		break;
	}
}

void nv::editor::TabManager::saveImpl(NodeData& child, VirtualFilesystem& vfs) {
	auto [json, byteCount] = child.editor.serialize();
	child.node = json.get<BufferedNode>();
	child.nodeJson = std::move(json);
	child.saved = true;
	vfs.dumpFileContents(child.editor.getID(), child.nodeJson.dump(2));

	const auto& dependantFileIDs = vfs.getDependantFiles(child.editor.getID());
	for (const auto& depFileID : dependantFileIDs) {
		auto& parent = m_nodeTabs.at(depFileID);
		parent.editor.updateNode(child.editor.getID(), *child.node);
		saveImpl(parent, vfs);
	}
}

bool nv::editor::TabManager::loadNode(VirtualFilesystem& vfs, FileID fileID,
	ErrorPopup& errorPopup)
{
	try {
		auto path = vfs.getNodePath(fileID);
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

bool nv::editor::TabManager::switchToNodeTab(VirtualFilesystem& vfs, FileID fileID, 
	ErrorPopup& errorPopup) 
{
	auto currNodeIt = m_nodeTabs.find(fileID);
	if (currNodeIt != m_nodeTabs.end()) {
		m_currNodeTab = &currNodeIt->second.editor;
		return true;
	} else {
		return loadNode(vfs, fileID, errorPopup);
	}
}

bool nv::editor::TabManager::loadTexture(VirtualFilesystem& vfs, FileID fileID, ErrorPopup& errorPopup) {
	auto instance = getGlobalInstance();
	try {
		auto tex = instance->registry.loadTexture(instance->getRenderer(), vfs.getFilePath(fileID).string());
		auto [it, tab] = m_texTabs.emplace(fileID, std::move(tex));
		m_currTexTab = &it->second;
		return true;
	} catch (const std::exception& e) {
		errorPopup.add(std::format("Error: {}", e.what()));
	}
	return false;
}

bool nv::editor::TabManager::switchToTextureTab(VirtualFilesystem& vfs, FileID fileID, 
	ErrorPopup& errorPopup)
{
	auto currNodeIt = m_texTabs.find(fileID);
	if (currNodeIt != m_texTabs.end()) {
		m_currTexTab = &currNodeIt->second;
		return true;
	} else {
		return loadTexture(vfs, fileID, errorPopup);
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

void nv::editor::TabManager::addNode(VirtualFilesystem& vfs, FileID id, ErrorPopup& errorPopup) {
	switchToNodeTab(vfs, id, errorPopup);
}

void nv::editor::TabManager::removeNode(FileID id) {
	if (m_currNodeTab && m_currNodeTab->getID() == id) {
		m_currNodeTab = nullptr;
	}
	m_nodeTabs.erase(id);
}

void nv::editor::TabManager::showTabs(VirtualFilesystem& vfs, ErrorPopup& errorPopup) {
	if (ImGui::BeginTabBar("Tabs")) {
		for (auto& [id, type] : m_tabs) {
			ImGui::PushID(getTemporaryImGuiID());
			
			if (ImGui::BeginTabItem(vfs.getFilename(id).c_str())) {
				switch (type) {
				case File::Type::Node:
					switchToNodeTab(vfs, id, errorPopup);
					break;
				case File::Type::Image:
					switchToTextureTab(vfs, id, errorPopup);
					break;
				}
				ImGui::EndTabItem();
			}
			ImGui::PopID();
		}
		ImGui::EndTabBar();
	}
}

void nv::editor::TabManager::updateCurrentTab(VirtualFilesystem& vfs, ErrorPopup& errorPopup) {
	auto selectedTab = vfs.getSelectedFile();
	if (!selectedTab) {
		m_currTexTab = nullptr;
		m_currTexTab = nullptr;
		return;
	}
	if (selectedTab->id != m_currTab.id && selectedTab->id != m_switchedToInvalidTabID) {
		switchTabs(*selectedTab, vfs, errorPopup);
	}
}

void nv::editor::TabManager::save(VirtualFilesystem& vfs) {
	for (auto& [id, tab] : m_nodeTabs) {
		saveImpl(tab, vfs);
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
	ErrorPopup& errorPopup) 
{
	auto tabWindowPos = getTabWindowPos();
	auto tabWindowSize = getTabWindowSize();
	ImGui::SetNextWindowPos(tabWindowPos);
	ImGui::SetNextWindowSize(tabWindowSize);

	ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4{ 0.0f, 0.0f, 0.0f, 0.f });

	ImGui::Begin(TAB_WINDOW_NAME, nullptr, DEFAULT_WINDOW_FLAGS);

	showTabs(vfs, errorPopup);
	updateCurrentTab(vfs, errorPopup);
	showCurrentTab(renderer);

	ImGui::PopStyleColor();

	ImGui::End();
}

boost::optional<nv::editor::NodeEditor&> nv::editor::TabManager::getCurrentNodeTab() {
	if (m_currNodeTab) {
		return *m_currNodeTab;
	} else {
		return boost::none;
	}
}