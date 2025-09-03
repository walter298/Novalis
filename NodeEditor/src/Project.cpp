#include <chrono>
#include <novalis/detail/ScopeExit.h>
#include <novalis/detail/serialization/AutoSerialization.h>
#include <novalis/detail/serialization/BoostContainerSerialization.h>
#include <novalis/detail/serialization/BufferedNodeSerialization.h>

#include "Project.h"
#include "ToolDisplay.h"

namespace {
	constexpr const char* FILE_MANAGER_KEY = "File_Manager";
	constexpr const char* FILE_ID_OFFSET_KEY = "File_ID_Offset";
	constexpr const char* DIRECTORY_ID_OFFSEY_KEY = "Directory_ID_Offset";
	constexpr const char* NAME_KEY = "Name";
}

void nv::editor::Project::loadFilesystemVersion(size_t projectIndex) {
	auto fsJsonPath = pfm.getFilesystemJSONPath(projectIndex);
	std::ifstream file{ fsJsonPath };
	assert(file.is_open());
	auto fsJson = nlohmann::json::parse(file);
	vfs = fsJson.get<VirtualFilesystem>();
	int x = 0;
}

void nv::editor::Project::writeFilesystemJson() {
	std::ofstream fsFileFile{ pfm.getFilesystemJSONPath(m_currProjectVersion) };
	nlohmann::json fsJson = vfs;
	fsFileFile << fsJson.dump(2);
}

void nv::editor::Project::writeGlobalProjectJson() {
	//update project json
	nlohmann::json globalProjectJson;
	globalProjectJson[FILE_MANAGER_KEY] = pfm;
	globalProjectJson[FILE_ID_OFFSET_KEY] = FileID::IDCount;
	globalProjectJson[DIRECTORY_ID_OFFSEY_KEY] = DirectoryID::IDCount;
	globalProjectJson[NAME_KEY] = m_name;

	//write project json
	std::ofstream projectJson{ pfm.getGlobalProjectFilePath() };
	assert(projectJson.is_open());
	projectJson << globalProjectJson.dump(2);
}

void nv::editor::Project::saveImpl(TabManager::NodeTab& tab, ErrorPopup& errorPopup) {
	auto [json, byteCount] = tab.editor.serialize();
	tab.node = json.get<BufferedNode>();
	tab.nodeJson = std::move(json);
	tab.saved = true;
	pfm.updateNodeFile(m_currProjectVersion, tab.editor.getID(), tab.nodeJson.dump(2));

	const auto& dependantFileIDs = vfs.getDependantFiles(tab.editor.getID());
	for (const auto& depFileID : dependantFileIDs) {
		auto& file = vfs.getFile(depFileID);
		if (file.getType() != File::Type::Node) {
			continue;
		}
		auto parentRes = tabManager.getNodeTab(pfm, m_currProjectVersion, depFileID, errorPopup);
		if (!parentRes) {
			continue;
		}
		parentRes->editor.updateNode(tab.editor.getID(), *tab.node);
		saveImpl(*parentRes, errorPopup);
	}
}

nv::editor::Project::Project(std::filesystem::path rootDirPath, std::string name) 
	: pfm{ rootDirPath }, m_name{ std::move(name) }
{
	m_currProjectVersion = pfm.addProjectVersion();
	writeFilesystemJson();
	writeGlobalProjectJson();
}

nv::editor::FileID::IntegerType nv::editor::Project::getFileIDOffset() const noexcept {
	return m_fileIDOffset;
}

nv::editor::DirectoryID::IntegerType nv::editor::Project::getDirectoryIDOffset() const noexcept {
	return m_directoryIDOffset;
}

bool nv::editor::Project::switchVersion(bool& cancelled, ErrorPopup& errorPopup) {
	assert(!cancelled);
	auto versionIndexRes = pfm.showVersionSelector(m_currProjectVersion, cancelled);
	if (cancelled) {
		return false;
	}
	if (versionIndexRes) {
		auto versionIndex = *versionIndexRes;
		assert(versionIndex != m_currProjectVersion); //should be disabled in selector
		tabManager.clear();
		loadFilesystemVersion(versionIndex);
		m_currProjectVersion = versionIndex;
		return true;
	}
	return false;
}

void nv::editor::Project::show(SDL_Renderer* renderer, ErrorPopup& errorPopup) {
	tabManager.show(renderer, vfs, pfm, m_currProjectVersion, errorPopup);
	vfs.show(tabManager, pfm, m_currProjectVersion, errorPopup);

	auto currTab = tabManager.getCurrentNodeTab();
	if (currTab && !currTab->hasNoLayers()) {
		showToolDisplay(currTab->isBusy());
	}
}

bool nv::editor::Project::save(ErrorPopup& errorPopup) {
	if (!tabManager.saveable(vfs, errorPopup)) {
		return false;
	}

	try {
		m_currProjectVersion = pfm.forkProjectVersion(m_currProjectVersion);
	} catch (const std::filesystem::filesystem_error& e) {
		errorPopup.add(std::format("Error: {}", e.what()));
		return false;
	}

	m_fileIDOffset = FileID::IDCount;
	m_directoryIDOffset = DirectoryID::IDCount;
	
	for (auto& [id, nodeTab] : tabManager) {
		if (!nodeTab.saved) {
			saveImpl(nodeTab, errorPopup);
		}
	}

	writeGlobalProjectJson();
	writeFilesystemJson();

	return true;
}

size_t nv::editor::Project::getCurrentVersion() const noexcept {
	return m_currProjectVersion;
}

const std::string& nv::editor::Project::getName() const noexcept {
	return m_name;
}

void nv::editor::from_json(const nlohmann::json& j, Project& project) {
	FileID::IDCount = j[FILE_ID_OFFSET_KEY].get<FileID::IntegerType>();
	DirectoryID::IDCount = j[DIRECTORY_ID_OFFSEY_KEY].get<DirectoryID::IntegerType>();
	project.pfm = j[FILE_MANAGER_KEY].get<ProjectFileManager>();
	project.m_name = j[NAME_KEY].get<std::string>();

	auto latestVersion = project.pfm.getCurrentVersionCount() - 1;
	project.loadFilesystemVersion(latestVersion);
	project.m_currProjectVersion = latestVersion;
}