#include "NodeTabList.h"
#include "Project.h"

boost::optional<nv::editor::NodeEditor&> nv::editor::Project::getCurrentTab() {
	return m_nodeManager.getCurrentTab();
}

void nv::editor::Project::showTabs() {
	m_nodeManager.showTabs(vfs);
}

void nv::editor::Project::showFilesystem(ErrorPopup& errorPopup) {
	vfs.show(m_nodeManager, errorPopup);
}

constexpr const char* ROOT_DIRECTORY_KEY = "Root_Directory";
constexpr const char* FILESYSTEM_KEY = "Virtual_Filesystem";
constexpr const char* NODE_GRAPH_KEY = "Node_Graph";
static constexpr const char* FILE_ID_OFFSET_KEY = "File_ID_Offset";
static constexpr const char* DIRECTORY_ID_OFFSEY_KEY = "Directory_ID_Offset";

void nv::editor::to_json(nlohmann::json& j, const Project& project) {
	j[ROOT_DIRECTORY_KEY] = project.m_rootDirectory;
	j[FILESYSTEM_KEY] = project.vfs;
	j[FILE_ID_OFFSET_KEY] = FileID::IDCount;
	j[DIRECTORY_ID_OFFSEY_KEY] = DirectoryID::IDCount;
}

void nv::editor::from_json(const nlohmann::json& j, Project& project) {
	project.m_rootDirectory = j[ROOT_DIRECTORY_KEY].get<std::string>();
	from_json(j[FILESYSTEM_KEY], project.vfs);
	FileID::IDCount = j[FILE_ID_OFFSET_KEY].get<FileID::IntegerType>();
	DirectoryID::IDCount = j[DIRECTORY_ID_OFFSEY_KEY].get<DirectoryID::IntegerType>();
}
