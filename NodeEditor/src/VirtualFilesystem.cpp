#include <algorithm>
#include <magic_enum/magic_enum.hpp>
#include <novalis/detail/file/File.h>
#include <novalis/detail/serialization/AutoSerialization.h>
#include <novalis/detail/ScopeExit.h>

#include "ProjectFileManager.h"
#include "TabManager.h"
#include "VirtualFilesystem.h"
#include "imgui/imgui.h"
#include "imgui/imgui_stdlib.h"

//for static methods
using namespace nv;
using namespace editor;

nv::editor::VirtualFilesystem::VirtualFilesystem()
	: m_fileDialog{ m_rootDirectoryID }, m_multiFileDialog{ m_rootDirectoryID }
{
	m_directories.emplace(m_rootDirectoryID, Directory::makeRoot());
	assert(DirectoryID::IDCount != m_rootDirectoryID.get());
}

const FileSet& nv::editor::VirtualFilesystem::getDependantFiles(FileID fileID) {
	return m_files.at(fileID).dependants;
}

nv::editor::FileID nv::editor::VirtualFilesystem::createNodeFile(const std::string& str) {
	return FileID{};
}

void nv::editor::VirtualFilesystem::showFile(FileID fileID, File& file, NameManager& parentNameManager) 
{
	if (m_renamingFileID == fileID) {
		bool renamed = false;
		file.show(parentNameManager, renamed);
		if (renamed) {
			m_renamingFileID = FileID::None();
		}
	} else {
		file.show();
	}
	if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
		m_selectedFileID = fileID;
	}
}

void nv::editor::VirtualFilesystem::deleteQueuedItems(TabManager& nodeManager, ErrorPopup& errorPopup) {
	if (m_deletedDirectoryID != DirectoryID::None()) {
		if (m_selectedFileID != FileID::None()) {
			auto currDirectory = m_files.at(m_selectedFileID).parent;
			if (isSubdirectory(currDirectory, m_deletedDirectoryID)) {
				m_selectedFileID = FileID::None();
			}
		}
		auto& deletedDir = m_directories.at(m_deletedDirectoryID);
		tryDeleteDirectory(m_deletedDirectoryID, deletedDir, nodeManager, errorPopup);
		m_deletedDirectoryID = DirectoryID::None();
	}
	if (m_deletedFileID != FileID::None()) {
		if (m_deletedFileID == m_selectedFileID) {
			m_selectedFileID = FileID::None();
		}
		auto& deletedFile = m_files.at(m_deletedFileID);
		tryDelete(m_deletedFileID, deletedFile, nodeManager, errorPopup);
		m_deletedFileID = FileID::None();
	}
}

void nv::editor::VirtualFilesystem::uploadImage(DirectoryID parentDirID, Directory& parentDir, 
	ProjectFileManager& pfm, ErrorPopup& errorPopup) 
{
	auto filePathRes = openFile({ { "PNG's", "PNG" }, { "AVIF's", "AVIF" }, { "BMP's", "BMP" }, { "JPG's", "JPG" } });
	if (!filePathRes) {
		return;
	}
	
	FileID fileID;

	try {
		pfm.uploadSharedResource(fileID, *filePathRes);
	} catch (const std::filesystem::filesystem_error& e) {
		errorPopup.add(std::format("Error: {}", e.what()));
		return;
	}

	auto fileExtension = filePathRes->extension().string();
	fileExtension.erase(fileExtension.begin()); //remove the dot before file extension

	//parse the image type
	auto imageType = magic_enum::enum_cast<File::Type>(fileExtension, magic_enum::case_insensitive);
	assert(imageType && (*imageType & File::Type::Image));

	//create image file
	File file{ parentDir.nameManager, filePathRes->filename().string(), *imageType};
	file.parent = parentDirID;

	//finally add the image in
	parentDir.files.insert(fileID);
	m_files.emplace(fileID, std::move(file));
}

FileID nv::editor::VirtualFilesystem::createNodeFile(DirectoryID parentDirID, Directory& parentDir) {
	FileID nodeFileID;
	parentDir.files.insert(nodeFileID);

	File file{ parentDir.nameManager, File::Type::Node };
	file.parent = parentDirID;
	m_files.emplace(nodeFileID, std::move(file));
	setCurrentlyRenamedFile(nodeFileID);

	return nodeFileID;
}

void nv::editor::VirtualFilesystem::createDirectory(DirectoryID parentDirID, Directory& parentDir) {
	DirectoryID id;
	setCurrentlyRenamedDirectory(id);
	parentDir.children.insert(id);
	Directory child{ parentDir.nameManager };
	child.parent = parentDirID;
	m_directories.emplace(id, std::move(child));
}

void nv::editor::VirtualFilesystem::renameDirectory(Directory& dir) {
	bool finishedRenaming = false;
	auto& parentDirNameManager = m_directories.at(dir.parent).nameManager;
	dir.inputName(parentDirNameManager, finishedRenaming);
	if (finishedRenaming) {
		m_renamingDirectoryID = DirectoryID::None();
	}
}

void nv::editor::VirtualFilesystem::showFileRightClickMenu(FileID fileID, File& file, 
	TabManager& nodeManager, ErrorPopup& errorPopup) 
{
	ImGui::SetNextItemOpen(true, ImGuiCond_Once);
	auto id = getTemporaryImGuiID();
	auto idStr = std::to_string(id);
	if (ImGui::IsItemClicked(ImGuiMouseButton_Right)) {
		ImGui::OpenPopup(idStr.c_str());
	}
	if (!ImGui::BeginPopup(idStr.c_str())) {
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
	TabManager& tabManager, ProjectFileManager& pfm, size_t projectIndex, bool& deleted, 
	ErrorPopup& errorPopup) noexcept
{
	assert(!deleted);

	ImGui::SetNextItemOpen(true, ImGuiCond_Once);
	ImGui::OpenPopupOnItemClick(dir.getName().c_str(), ImGuiPopupFlags_MouseButtonRight);
	if (!ImGui::BeginPopup(dir.getName().c_str())) {
		return;
	}

	if (ImGui::MenuItem("New Directory")) {
		createDirectory(dirID, dir);
	}

	if (ImGui::MenuItem("New Node")) {
		auto fileID = createNodeFile(dirID, dir);
		tabManager.addNode(pfm, projectIndex, fileID, errorPopup);
	}
	if (ImGui::MenuItem("Upload Image")) {
		uploadImage(dirID, dir, pfm, errorPopup);
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

void nv::editor::VirtualFilesystem::showImpl(DirectoryID dirID, TabManager& tabManager, 
	ProjectFileManager& pfm, size_t projectIndex, ErrorPopup& errorPopup) 
{
	auto ret = FileID::None();

	auto& dir = m_directories.at(dirID);
	ImGui::SetNextItemOpen(true, ImGuiCond_Once);
	
	if (m_renamingDirectoryID == dirID) {
		renameDirectory(dir);
		return;
	}
	
	dropFile(dirID);

	dir.showIcon();
	ImGui::SameLine();

	ImGui::PushID(dir.getImGuiID());
	nv::detail::ScopeExit popID{ [] { ImGui::PopID(); } };

	if (ImGui::TreeNodeEx(dir.getName().c_str())) {
		dropFile(dirID);

		nv::detail::ScopeExit scopeExit{ [] {
			ImGui::TreePop();
		} };

		dir.open = true;
		bool dirDeleted = false;
		showDirectoryRightClickMenu(dirID, dir, tabManager, pfm, projectIndex, dirDeleted, errorPopup);
		if (dirDeleted) {
			return;
		}
		
		for (const auto& fileID : dir.files) {
			auto& file = m_files.at(fileID);
			showFile(fileID, file, dir.nameManager);
			dragFile(fileID, file);
			dropFile(dirID);
			showFileRightClickMenu(fileID, file, tabManager, errorPopup);
		}

		for (const auto& childDirID : dir.children) {
			showImpl(childDirID, tabManager, pfm, projectIndex, errorPopup);
		}
	} else {
		dir.open = false;
	}
}

void nv::editor::VirtualFilesystem::setCurrentlyRenamedDirectory(DirectoryID dirID) {
	m_renamingDirectoryID = dirID;
	m_renamingFileID = FileID::None();
}

void nv::editor::VirtualFilesystem::setCurrentlyRenamedFile(FileID fileID) {
	m_renamingFileID = fileID;
	m_renamingDirectoryID = DirectoryID::None();
}

constexpr const char* FILE_PAYLOAD_NAME = "Payload";

void nv::editor::VirtualFilesystem::dragFile(FileID draggedFileID, File& draggedFile) {
	if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)) {
		m_draggedFileID = draggedFileID;
		ImGui::SetDragDropPayload(FILE_PAYLOAD_NAME, &draggedFileID, sizeof(FileID));
		draggedFile.show();
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
			oldParent.nameManager.deleteName(file.getName());

			//replace file parent with dropped directory
			file.parent = dirID;
			auto& newParent = m_directories.at(dirID);
			newParent.files.emplace(droppedFileID);
			file.makeNameUnique(newParent.nameManager);
		}
		ImGui::EndDragDropTarget();
	}
}

bool nv::editor::VirtualFilesystem::tryDelete(FileID fileID, File& file, TabManager& nodeManager, 
	ErrorPopup& errorPopup) 
{
	if (file.dependants.size() > 0) {
		errorPopup.add(std::format("Error: cannot delete {}, as file is referenced by: ", file.getName()));
		for (const auto& [idx, dependantID] : std::views::enumerate(file.dependants)) {
			errorPopup.add(std::format("{}. {}", idx, m_files.at(dependantID).getName()));
		}
		return false;
	} else {
		for (const auto& dependencyID : file.dependencies) {
			m_files.at(dependencyID).dependants.erase(fileID);
		}
		auto& parent = m_directories.at(file.parent);
		parent.nameManager.deleteName(file.getName());
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
			auto& topLevelDirName = m_directories.at(dirID).getName();
			errorPopup.add(std::format("Error: cannot delete {}, as {} depends on {}", topLevelDirName,
				dependantFile.getName(), file.getName()));
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

void nv::editor::VirtualFilesystem::deleteDirectoryImpl(DirectoryID dirID, Directory& dir, TabManager& nodeManager) {
	for (const auto& fileID : dir.files) {
		m_files.erase(fileID);
		nodeManager.removeNode(fileID);
	}
	for (const auto& childDirID : dir.children) {
		deleteDirectoryImpl(childDirID, m_directories.at(childDirID), nodeManager);
	}
	auto& parent = m_directories.at(dir.parent);
	parent.children.erase(dirID);
	parent.nameManager.deleteName(dir.getName());
	m_directories.erase(dirID);
}

bool nv::editor::VirtualFilesystem::tryDeleteDirectory(DirectoryID dirID, Directory& dir, TabManager& nodeManager, 
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

std::optional<FileID> nv::editor::VirtualFilesystem::showFileDialog(const ProjectFileManager& pfm, 
	File::Type filter, bool& cancelled) 
{
	return m_fileDialog.show(pfm, m_directories, m_files, filter, cancelled);
}

std::optional<FileSet> nv::editor::VirtualFilesystem::showMultipleFileDialog(const ProjectFileManager& pfm, 
	File::Type filter, bool& cancelled) 
{
	return m_multiFileDialog.show(pfm, m_directories, m_files, filter, cancelled);
}

bool nv::editor::VirtualFilesystem::isTransitiveDependency(FileID fileID, FileID dependencyID) const {
	auto& dependencies = m_files.at(fileID).dependencies;
	for (const auto& currDependencyID : dependencies) {
		if (dependencies.contains(dependencyID) || isTransitiveDependency(currDependencyID, dependencyID)) {
			return true;
		}
	}
	return false;
}

bool nv::editor::VirtualFilesystem::createDependency(FileID fileID, FileID dependencyFileID) {
	//prevent circular dependencies
	if (isTransitiveDependency(dependencyFileID, fileID)) {
		return false;
	}

	auto& file = m_files.at(fileID);
	file.dependencies.insert(dependencyFileID);

	auto& dependencyFile = m_files.at(dependencyFileID);
	dependencyFile.dependants.insert(fileID);

	return true;
}

const File& nv::editor::VirtualFilesystem::getFile(FileID fileID) const {
	return m_files.at(fileID);
}

std::optional<nv::editor::Tab> nv::editor::VirtualFilesystem::getSelectedFile() const noexcept {
	if (m_selectedFileID == FileID::None()) {
		return std::nullopt;
	}
	auto& file = m_files.at(m_selectedFileID);
	return Tab{ m_selectedFileID, file.getType() };
}

void nv::editor::VirtualFilesystem::show(TabManager& nodeManager, ProjectFileManager& pfm, 
	size_t projectIndex, ErrorPopup& errorPopup) 
{
	ImGui::SetNextWindowPos(getFilesystemWindowPos());
	ImGui::SetNextWindowSize(getFilesystemWindowSize());
	ImGui::Begin(FILESYSTEM_WINDOW_NAME, nullptr, DEFAULT_WINDOW_FLAGS);

	showImpl(m_rootDirectoryID, nodeManager, pfm, projectIndex, errorPopup);
	deleteQueuedItems(nodeManager, errorPopup);

	ImGui::End();
}

const std::string& nv::editor::VirtualFilesystem::getFilename(FileID fileID) const noexcept {
	return m_files.at(fileID).getName();
}