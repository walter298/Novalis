#include <algorithm>
#include <magic_enum/magic_enum.hpp>
#include <novalis/detail/file/File.h>
#include <novalis/detail/serialization/AutoSerialization.h>
#include <novalis/detail/ScopeExit.h>

#include "VirtualFilesystem.h"
#include "imgui/imgui.h"
#include "imgui/imgui_stdlib.h"

//for static methods
using namespace nv;
using namespace editor;

nv::editor::VirtualFilesystem::VirtualFilesystem() :
	m_fileDialog{ m_rootDirectoryID }, m_multiFileDialog{ m_rootDirectoryID }
{
}

nv::editor::VirtualFilesystem::VirtualFilesystem(std::filesystem::path projectRootDirectoryPath)
	: m_projectRootDirectoryPath{ std::move(projectRootDirectoryPath) }, m_fileDialog{ m_rootDirectoryID },
	m_multiFileDialog{ m_rootDirectoryID }
{
	//create root directory
	auto it = m_directories.emplace(
		std::piecewise_construct,
		std::forward_as_tuple(m_rootDirectoryID), std::forward_as_tuple()
	);
	it.first->second.name = "root";
	
	//generate project sub-directories
	std::filesystem::create_directory(getAssetDirectoryPath());
	std::filesystem::create_directory(getNodeDirectoryPath());
}

nv::editor::FileID nv::editor::VirtualFilesystem::createNodeFile(const std::string& str) {
	return FileID{};
}

ImTextureID nv::editor::VirtualFilesystem::getFileIcon(std::string fileExtension) {
	static boost::unordered_flat_map<std::string, SDL_Texture*> fileIcons{
		{ "json", nodeFileIconTex.tex },
		{ "avif", avifFileIconTex.tex },
		{ "png", pngFileIconTex.tex },
		{ "jpg", jpgFileIconTex.tex },
		{ "bmp", bmpFileIconTex.tex } 
	};
	for (auto& chr : fileExtension) {
		chr = tolower(chr);
	}
	return reinterpret_cast<ImTextureID>(fileIcons.at(fileExtension));
}

void nv::editor::VirtualFilesystem::deleteQueuedItems(NodeManager& nodeManager, ErrorPopup& errorPopup) {
	if (m_deletedDirectoryID != DirectoryID::None()) {
		auto& deletedDir = m_directories.at(m_deletedDirectoryID);
		tryDeleteDirectory(m_deletedDirectoryID, deletedDir, nodeManager, errorPopup);
		m_deletedDirectoryID = DirectoryID::None();
	}
	if (m_deletedFileID != FileID::None()) {
		auto& deletedFile = m_files.at(m_deletedFileID);
		tryDelete(m_deletedFileID, deletedFile, nodeManager, errorPopup);
		m_deletedFileID = FileID::None();
	}
}

std::filesystem::path nv::editor::VirtualFilesystem::getAssetDirectoryPath() const {
	return m_projectRootDirectoryPath / "assets";
}

std::filesystem::path nv::editor::VirtualFilesystem::getNodeDirectoryPath() const {
	return m_projectRootDirectoryPath / "nodes";
}

void nv::editor::VirtualFilesystem::uploadImage(DirectoryID parentDirID, Directory& parentDir, 
	ErrorPopup& errorPopup) 
{
	auto filePathRes = openFile({ { "PNG's", "PNG" }, { "AVIF's", "AVIF" }, { "BMP's", "BMP" }, { "JPG's", "JPG" } });
	if (!filePathRes) {
		return;
	}
	std::ifstream originalFile{ *filePathRes, std::ios::binary };
	if (!originalFile.is_open()) {
		errorPopup.add(std::format("Error: can't open {}", *filePathRes));
		return;
	}
	
	//create unique file path based off image ID in the assets directory
	FileID imageFileID;
	auto fileExtensionRes = parseFileExtension(*filePathRes);
	assert(fileExtensionRes);
	auto copyPath = getAssetDirectoryPath() / (std::to_string(imageFileID) + '.' + *fileExtensionRes);

	std::ofstream fileCopy{ copyPath, std::ios::binary };
	if (!fileCopy.is_open()) {
		errorPopup.add(std::format("Error: can't open {}", copyPath.string()));
		return;
	}

	fileCopy << originalFile.rdbuf();

	//create image file
	File file;
	file.icon = getFileIcon(*fileExtensionRes);
	file.realPath = copyPath;
	file.type = File::Type::Image;
	file.name = nv::fileName(*filePathRes);
	file.parent = parentDirID;

	//finally add the image in
	parentDir.files.insert(imageFileID);
	m_files.emplace(imageFileID, std::move(file));
}

FileID nv::editor::VirtualFilesystem::createNodeFile(DirectoryID parentDirID, Directory& parentDir)
{
	constexpr const char* NODE_FILE_EXTENSION = "json";

	FileID nodeFileID;

	parentDir.files.insert(nodeFileID);
	File file;
	file.parent = parentDirID;
	file.type = File::Type::Node;
	file.icon = getFileIcon(NODE_FILE_EXTENSION);
	file.realPath = getNodeDirectoryPath() / std::to_string(nodeFileID) / "." / NODE_FILE_EXTENSION;
	m_files.emplace(nodeFileID, std::move(file));
	setCurrentlyRenamedFile(nodeFileID);

	return nodeFileID;
}

void nv::editor::VirtualFilesystem::createDirectory(DirectoryID parentDirID) {
	DirectoryID id;
	setCurrentlyRenamedDirectory(id);
	m_directories.at(parentDirID).children.insert(id);
	Directory child;
	child.parent = parentDirID;
	m_directories.emplace(id, std::move(child));
}

void nv::editor::VirtualFilesystem::createNodeFile(NodeManager& nodeManager) {
	FileID id;
	nodeManager.addNode(id);
}

void nv::editor::VirtualFilesystem::showFileRightClickMenu(FileID fileID, File& file, 
	NodeManager& nodeManager, bool& deleted, ErrorPopup& errorPopup) 
{
	ImGui::SetNextItemOpen(true, ImGuiCond_Once);
	ImGui::OpenPopupOnItemClick(file.name.c_str(), ImGuiPopupFlags_MouseButtonRight);
	if (!ImGui::BeginPopup(file.name.c_str())) {
		return;
	}
	if (ImGui::MenuItem("Rename")) {
		setCurrentlyRenamedFile(fileID);
	}
	if (ImGui::MenuItem("Delete")) {
		m_deletedFileID = fileID;
	}
	ImGui::EndPopup();
}

void nv::editor::VirtualFilesystem::showDirectoryRightClickMenu(DirectoryID dirID, Directory& dir, 
	NodeManager& nodeManager, bool& deleted, ErrorPopup& errorPopup) noexcept
{
	assert(!deleted);

	ImGui::SetNextItemOpen(true, ImGuiCond_Once);
	ImGui::OpenPopupOnItemClick(dir.name.c_str(), ImGuiPopupFlags_MouseButtonRight);
	if (!ImGui::BeginPopup(dir.name.c_str())) {
		return;
	}

	if (ImGui::MenuItem("New Directory")) {
		createDirectory(dirID);
	}

	if (ImGui::MenuItem("New Node")) {
		auto fileID = createNodeFile(dirID, dir);
		nodeManager.addNode(fileID);
	}
	if (ImGui::MenuItem("Upload Image")) {
		uploadImage(dirID, dir, errorPopup);
	}
	//disallow renaming of the root directory because there is no parent name manager
	if (dirID != m_rootDirectoryID && ImGui::MenuItem("Rename")) {
		m_renamingDirectoryID = dirID;
	}
	//disallow deleting the root because we wouldn't be able to add it back afterwards!
	if (dirID != m_rootDirectoryID && ImGui::MenuItem("Delete")) {
		m_deletedDirectoryID = dirID;
	}

	ImGui::EndPopup();
}

void nv::editor::VirtualFilesystem::showImpl(DirectoryID dirID, NodeManager& nodeManager, ErrorPopup& errorPopup) {
	auto& dir = m_directories.at(dirID);
	ImGui::PushID(dir.imguiID);
	ImGui::SetNextItemOpen(true, ImGuiCond_Once);

	if (m_renamingDirectoryID == dirID) {
		renameDirectory(dir);
		ImGui::PopID();
		return;
	}

	auto& folderTex = dir.open ? openFolderTex : closedFolderTex;
	ImGui::Image(reinterpret_cast<ImTextureID>(folderTex.tex), DIRECTORY_ICON_SIZE);

	dropFile(dirID);

	ImGui::SameLine();
	if (ImGui::TreeNodeEx(dir.name.c_str())) {
		dropFile(dirID);

		nv::detail::ScopeExit scopeExit{ [] {
			ImGui::TreePop();
		} };
		dir.open = true;

		bool dirDeleted = false;
		showDirectoryRightClickMenu(dirID, dir, nodeManager, dirDeleted, errorPopup);
		if (!dirDeleted) {
			bool fileDeleted = false;

			for (const auto& fileID : dir.files) {
				auto& file = m_files.at(fileID);
				showFile(dir.nameManager, fileID, file);
				dragFile(fileID, file, dirID);
				dropFile(dirID);
				showFileRightClickMenu(fileID, file, nodeManager, fileDeleted, errorPopup);
			}

			for (const auto& childDirID : dir.children) {
				showImpl(childDirID, nodeManager, errorPopup);
			}
		}
	} else {
		dir.open = false;
	}

	ImGui::PopID();
}

void nv::editor::VirtualFilesystem::setCurrentlyRenamedDirectory(DirectoryID dirID) {
	m_renamingDirectoryID = dirID;
	m_renamingFileID = FileID::None();
}

void nv::editor::VirtualFilesystem::setCurrentlyRenamedFile(FileID fileID) {
	m_renamingFileID = fileID;
	m_renamingDirectoryID = DirectoryID::None();
}

void nv::editor::VirtualFilesystem::renameDirectory(Directory& dir) {
	assert(m_renamingDirectoryID != m_rootDirectoryID);
	auto& parentDir = m_directories.at(dir.parent);
	if (parentDir.nameManager.inputName("", dir.name)) {
		m_renamingDirectoryID = DirectoryID::None();
	}
}

void nv::editor::VirtualFilesystem::showFile(NameManager& dirNameManager, FileID fileID, File& file) {
	ImGui::Image(file.icon, FILE_ICON_SIZE);
	ImGui::SameLine();

	ImGui::PushID(file.imguiID);
	if (fileID == m_renamingFileID) {
		if (dirNameManager.inputName("", file.name)) {
			m_renamingFileID = FileID::None();
		}
	} else {
		ImGui::TextUnformatted(file.name.c_str());
	}
	ImGui::PopID();
}

constexpr const char* FILE_PAYLOAD_NAME = "Payload";

void nv::editor::VirtualFilesystem::dragFile(FileID draggedFileID, File& draggedFile, DirectoryID dirID) {
	if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)) {
		m_draggedFileID = draggedFileID;
		ImGui::SetDragDropPayload(FILE_PAYLOAD_NAME, &draggedFileID, sizeof(FileID));
		ImGui::Image(draggedFile.icon, FILE_ICON_SIZE);
		ImGui::SameLine();
		ImGui::TextUnformatted(draggedFile.name.c_str());
		ImGui::EndDragDropSource();
	}
}

void nv::editor::VirtualFilesystem::dropFile(DirectoryID dirID) {
	if (ImGui::BeginDragDropTarget()) {
		auto payload = ImGui::AcceptDragDropPayload(FILE_PAYLOAD_NAME);
		if (payload) {
			auto droppedFileID = *(reinterpret_cast<FileID*>(payload->Data));

			//remove file from old directory
			auto& file = m_files.at(droppedFileID);
			auto& oldParent = m_directories.at(file.parent);
			oldParent.files.erase(droppedFileID);

			//replace file parent with dropped directory
			file.parent = dirID;
			auto& newParent = m_directories.at(dirID);
			newParent.files.emplace(droppedFileID);
		}
		ImGui::EndDragDropTarget();
	}
}

bool nv::editor::VirtualFilesystem::tryDelete(FileID fileID, File& file, NodeManager& nodeManager, 
	ErrorPopup& errorPopup) 
{
	if (file.dependants.size() > 0) {
		errorPopup.add(std::format("Error: cannot delete {}, as file is referenced by: ", file.name));
		for (const auto& [idx, dependantID] : std::views::enumerate(file.dependants)) {
			errorPopup.add(std::format("{}. {}", idx, m_files.at(dependantID).name));
		}
		return false;
	} else {
		for (const auto& dependencyID : file.dependencies) {
			m_files.at(dependencyID).dependants.erase(fileID);
		}
		auto& parent = m_directories.at(file.parent);
		parent.files.erase(fileID);
		m_files.erase(fileID);
		nodeManager.removeNode(fileID);
		return true;
	}
	return true;
}

bool VirtualFilesystem::isDependencyForFilesOutsideDirectory(DirectoryID dirID, const File& file, 
	ErrorPopup& errorPopup) const
{
	for (const auto& dependantFileID : file.dependants) {
		auto& dependantFile = m_files.at(dependantFileID);
		auto dependantDirectoryID = dependantFile.parent;

		if (!isSubdirectory(dependantDirectoryID, dirID)) {
			auto& topLevelDirName = m_directories.at(dirID).name;
			errorPopup.add(std::format("Error: cannot delete {}, as {} depends on {}", topLevelDirName,
				dependantFile.name, file.name));
			return true;
		}
	}
	return false;
}

bool nv::editor::VirtualFilesystem::isDirectoryDeletable(DirectoryID topLevelDirID, DirectoryID subdirectoryID, 
	ErrorPopup& errorPopup) const 
{
	auto& subdirectory = m_directories.at(subdirectoryID);

	for (const auto& fileID : subdirectory.files) {
		auto& file = m_files.at(fileID);
		if (isDependencyForFilesOutsideDirectory(topLevelDirID, file, errorPopup)) {
			return false;
		}
	}
	for (const auto& childDirID : subdirectory.children) {
		if (!isDirectoryDeletable(topLevelDirID, childDirID, errorPopup)) {
			return false;
		}
	}
	return true;
}

void nv::editor::VirtualFilesystem::deleteDirectoryImpl(DirectoryID dirID, Directory& dir, NodeManager& nodeManager) {
	for (const auto& fileID : dir.files) {
		m_files.erase(fileID);
		nodeManager.removeNode(fileID);
	}
	for (const auto& childDirID : dir.children) {
		deleteDirectoryImpl(childDirID, m_directories.at(childDirID), nodeManager);
	}
	auto& parent = m_directories.at(dir.parent);
	parent.children.erase(dirID);
	parent.nameManager.deleteName(dir.name);
	m_directories.erase(dirID);
}

bool nv::editor::VirtualFilesystem::tryDeleteDirectory(DirectoryID dirID, Directory& dir, NodeManager& nodeManager, 
	ErrorPopup& errorPopup) 
{
	if (!isDirectoryDeletable(dirID, dirID, errorPopup)) {
		return false;
	}
	deleteDirectoryImpl(dirID, dir, nodeManager);
	return true;
}

bool nv::editor::VirtualFilesystem::isSubdirectory(DirectoryID childDirID, DirectoryID parentDirID) const {
	if (childDirID == parentDirID) {
		return true;
	}
	if (childDirID == m_rootDirectoryID) {
		return false;
	}
	return isSubdirectory(m_directories.at(childDirID).parent, parentDirID);
}

std::optional<FileID> nv::editor::VirtualFilesystem::showFileDialog(File::Type filter, bool& cancelled) {
	return m_fileDialog.show(m_directories, m_files, filter, cancelled);
}

std::optional<FileSet> nv::editor::VirtualFilesystem::showMultipleFileDialog(File::Type filter, bool& cancelled) {
	return m_multiFileDialog.show(m_directories, m_files, filter, cancelled);
}

nv::detail::TexturePtr nv::editor::VirtualFilesystem::getTexture(FileID fileID) {
	auto& file = m_files.at(fileID);
	assert(file.type == File::Type::Image);

	auto instance = getGlobalInstance();
	auto tex = instance->registry.loadTexture(instance->getRenderer(), file.realPath.string());

	return tex;
}

FileID nv::editor::VirtualFilesystem::saveImage(SDL_Surface* surface, ImageType imageType) {
	FileID id;
	auto enumName = magic_enum::enum_name(imageType);
	auto path = getAssetDirectoryPath() / (std::to_string(id) + "." + std::string{ enumName.data(), enumName.size() });

	switch (imageType) {
	case PNG:
		IMG_SavePNG(surface, path.string().c_str());
		break;
	case JPG:
		IMG_SaveJPG(surface, path.string().c_str(), 100);
		break;
	case BMP:
		SDL_SaveBMP(surface, path.string().c_str());
		break;
	case AVIF:
		IMG_SaveAVIF(surface, path.string().c_str(), 100);
		break;
	}

	return id;
}

void nv::editor::VirtualFilesystem::dumpFileContents(FileID fileID, const std::string& text) {
	auto& virtualFile = m_files.at(fileID);
	std::ofstream file{ virtualFile.realPath };
	assert(file.is_open());
	file << text;
}

void nv::editor::VirtualFilesystem::createDependency(FileID fileID, FileID dependencyFileID) {
	auto& file = m_files.at(fileID);
	file.dependencies.insert(dependencyFileID);

	auto& dependencyFile = m_files.at(dependencyFileID);
	dependencyFile.dependants.insert(fileID);
}

void nv::editor::VirtualFilesystem::loadFolderTextures(SDL_Renderer* renderer) noexcept {
#if _DEBUG
	static bool loaded = false;
	assert(!loaded);
#endif
	static constexpr const char* rootDirEnvVar = "NOVALIS_ROOT";
	auto rootDirPath = std::getenv(rootDirEnvVar);

	namespace fs = std::filesystem;
	using namespace std::literals;

	auto folderImageDir = rootDirPath + "/NodeEditor/novalis_assets/file_explorer_images/"s;

	auto loadTexture = [&](std::string relativeImagePath) -> nv::detail::TexturePtr {
		auto path = folderImageDir + relativeImagePath;
		return { renderer, path.c_str() };
	};
	openFolderTex   = loadTexture("open_folder.png");
	closedFolderTex = loadTexture("closed_folder.png");
	nodeFileIconTex = loadTexture("node_file_icon.png");
	pngFileIconTex  = loadTexture("png_file_icon.png");
	jpgFileIconTex  = loadTexture("jpg_file_icon.png");
	bmpFileIconTex  = loadTexture("bmp_file_icon.png");
	avifFileIconTex = loadTexture("avif_file_icon.png");
}

void nv::editor::VirtualFilesystem::destroyFolderTextures() {
#if _DEBUG
	static bool destroyed = false;
	assert(!destroyed);
#endif
	auto destroy = [](nv::detail::TexturePtr& tex) {
		SDL_DestroyTexture(tex.tex);
		tex.tex = nullptr;
	};
	destroy(openFolderTex);
	destroy(closedFolderTex);
	destroy(nodeFileIconTex);
	destroy(pngFileIconTex);
	destroy(jpgFileIconTex);
	destroy(bmpFileIconTex);
	destroy(avifFileIconTex);
}

void nv::editor::VirtualFilesystem::show(NodeManager& nodeManager, ErrorPopup& errorPopup) {
	ImGui::SetNextWindowPos(getFilesystemWindowPos());
	ImGui::SetNextWindowSize(getFilesystemWindowSize());
	ImGui::Begin(FILESYSTEM_WINDOW_NAME, nullptr, DEFAULT_WINDOW_FLAGS);

	showImpl(m_rootDirectoryID, nodeManager, errorPopup);
	deleteQueuedItems(nodeManager, errorPopup);

	ImGui::End();
}

const std::string& nv::editor::VirtualFilesystem::getFilename(FileID fileID) const noexcept {
	return m_files.at(fileID).name;
}

static constexpr const char* FILE_TREE_KEY = "File_Tree";
static constexpr const char* ROOT_DIRECTORY_ID_KEY = "Root_Directory_ID";
static constexpr const char* DIRECTORY_NAME_KEY = "Directory_Name";
static constexpr const char* CHILDREN_KEY = "Children";
static constexpr const char* FILE_IDS_KEY = "File_IDs";
static constexpr const char* DIRECTORY_IDS_KEY = "Directory_IDs";

void nv::editor::VirtualFilesystem::saveDirectory(const Directory& directory, nlohmann::json& j) const {
	j[DIRECTORY_NAME_KEY] = directory.name;
	j[FILE_IDS_KEY] = directory.files;
	j[DIRECTORY_IDS_KEY] = directory.children;
	auto& childrenJson = j[CHILDREN_KEY] = nlohmann::json::array();
	for (const auto& childID : directory.children) {
		nlohmann::json childJson;
		saveDirectory(m_directories.at(childID), childJson);
		childrenJson.push_back(std::move(childJson));
	}
}

void nv::editor::VirtualFilesystem::loadFromJsonImpl(const json& dirJson, DirectoryID dirID) {
	Directory dir;
	dir.name = dirJson[DIRECTORY_NAME_KEY].get<std::string>();
	dir.files = dirJson[FILE_IDS_KEY].get<FileSet>();
	dir.children = dirJson[DIRECTORY_IDS_KEY].get<DirectorySet>();
	auto& childrenJson = dirJson[CHILDREN_KEY];
	m_directories.emplace(dirID, std::move(dir));
	for (const auto& [childJson, childDirID] : std::views::zip(childrenJson, dir.children)) {
		loadFromJsonImpl(childJson, childDirID);
	}
}

void nv::editor::VirtualFilesystem::dumpJson(json& j) const {
	saveDirectory(m_directories.at(m_rootDirectoryID), j[FILE_TREE_KEY]);
	j[ROOT_DIRECTORY_ID_KEY] = m_rootDirectoryID;
}

void nv::editor::VirtualFilesystem::loadFromJson(const json& j) {
	m_rootDirectoryID = j[ROOT_DIRECTORY_ID_KEY].get<DirectoryID>();
	loadFromJsonImpl(j[FILE_TREE_KEY], m_rootDirectoryID);
}

void nv::editor::to_json(json& j, const VirtualFilesystem& vfs) {
	vfs.dumpJson(j);
}

void nv::editor::from_json(const json& j, VirtualFilesystem& vfs) {
	vfs.loadFromJson(j);
}