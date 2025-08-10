#include "Project.h"
#include "ToolDisplay.h"

void nv::editor::Project::show(SDL_Renderer* renderer, ErrorPopup& errorPopup) {
	tabManager.show(renderer, vfs, errorPopup);
	vfs.show(tabManager, errorPopup);

	auto currTab = tabManager.getCurrentNodeTab();
	if (currTab && !currTab->hasNoLayers()) {
		showToolDisplay(currTab->isBusy());
	}
}

void nv::editor::Project::save(ErrorPopup& errorPopup) {
	if (!tabManager.saveable(vfs, errorPopup)) {
		return;
	}

	auto projectFilePath = m_rootDirectory / "project.json";
	std::ofstream file{ projectFilePath };
	if (!file.is_open()) {
		errorPopup.add("Could not open project file: " + projectFilePath.string());
	} else {
		tabManager.save(vfs);
		auto projectJson = nlohmann::json::object();
		to_json(projectJson, *this);
		assert(!projectJson.is_array());
		file << projectJson.dump(2);
	}
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