#include <novalis/Instance.h>
#include <novalis/detail/ScopeExit.h>

#include "FileDialog.h"
#include "ProjectFileManager.h"
#include "VirtualFilesystem.h"
#include "WindowLayout.h"

//for static methods
using namespace nv;
using namespace editor;

namespace {
	constexpr ImVec4 DIALOG_BACKGROUND_COLOR{ 0.58f, 0.58f, 0.58f, 1.0f };
	
	struct ItemClickResult {
		bool clicked = false;
		bool doubledClicked = false;
	};
	ItemClickResult showIcon(ImTextureID tex, const std::string& name, ImVec2 texSize, bool isSelected) {
		ItemClickResult res;
		
		ImGui::Image(tex, FILE_ICON_SIZE);
		ImGui::SameLine();

		auto textPos = ImGui::GetCursorScreenPos();
		auto textSize = ImGui::CalcTextSize(name.c_str());
	
		ImGui::PushID(getTemporaryImGuiID());
		ImGui::InvisibleButton("invis", textSize);
		
		auto buttonMin = ImGui::GetItemRectMin();
		auto buttonMax = ImGui::GetItemRectMax();
		auto drawList = ImGui::GetWindowDrawList();
		if (isSelected) {
			drawList->AddRectFilled(buttonMin, buttonMax, IM_COL32(0, 210, 0, 64));
		}
		ImGui::SetItemAllowOverlap();
		ImGui::SetCursorScreenPos(textPos);
		ImGui::TextUnformatted(name.c_str());
		ImGui::PopID();

		if (ImGui::IsItemClicked()) {
			res.clicked = true;
		}
		if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
			res.doubledClicked = true;
		}

		return res;
	}

	ImTextureID getFileTex(FileID fileID, const File& file, const ProjectFileManager& pfm) {
		if (static_cast<int>((file.getType() & File::Type::Image)) != 0) {
			auto instance = getGlobalInstance();
			auto filePath = pfm.getSharedAssetPath(fileID).string();
			auto tex = instance->registry.loadTexture(instance->getRenderer(), filePath);
			return reinterpret_cast<ImTextureID>(tex.tex);
		} else {
			return file.getIcon();
		}
	}
}

void nv::editor::FileDialogBase::showDirectoryStack(const DirectoryMap& directories) {
	auto it = m_directoryStack.begin();
	for (; it != m_directoryStack.end(); it++) {
		auto& dirID = *it;
		auto& dirName = directories.at(dirID).getName();
		ImGui::PushID(getTemporaryImGuiID());
		nv::detail::ScopeExit exit{ [] { ImGui::PopID(); } };
		if (ImGui::Button(dirName.c_str())) {
			m_currDirectoryID = dirID;
			break;
		}
		ImGui::SameLine();
	}
	ImGui::NewLine();

	if (it != m_directoryStack.end()) {
		m_directoryStack.erase(std::next(it), m_directoryStack.end());
	}
}

void nv::editor::FileDialogBase::showFiles(const ProjectFileManager& pfm, const FileSet& displayedFiles, const FileMap& fileMap, 
	File::Type filter) 
{
	for (const auto& fileID : displayedFiles) {
		auto& file = fileMap.at(fileID);
		if ((file.getType() & filter) == 0) {
			continue;
		}

		showIcon(getFileTex(fileID, file, pfm), file.getName(), FILE_ICON_SIZE, fileID == m_currClickedFileID);
		if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
			m_currClickedFileID = fileID;
		}
	}
}

void nv::editor::FileDialogBase::showDirectories(const DirectorySet& displayedDirectories, 
	const DirectoryMap& dirMap) 
{
	for (const auto& dirID : displayedDirectories) {
		auto clickRes = showIcon(getFolderIcon(), dirMap.at(dirID).getName(), FILE_ICON_SIZE, m_currClickedDirectoryID == dirID);
		if (clickRes.clicked) {
			m_currClickedDirectoryID = dirID;
		}
		if (clickRes.doubledClicked) {
			m_currDirectoryID = dirID;
			m_directoryStack.push_back(dirID);
			break;
		}
	}
}

FileID nv::editor::FileDialogBase::getClickedFileID() const noexcept {
	return m_currClickedFileID;
}

void nv::editor::FileDialogBase::showFilesAndDirectories(const ProjectFileManager& pfm,
	const DirectoryMap& dirMap, const FileMap& fileMap, File::Type filter) 
{
	showDirectoryStack(dirMap);
	ImGui::NewLine();

	const auto& directory = dirMap.at(m_currDirectoryID);
	showFiles(pfm, directory.files, fileMap, filter);
	showDirectories(directory.children, dirMap);
}

void nv::editor::FileDialogBase::showCreateButton(bool& clickedCreate, bool disabled) {
	ImGui::BeginDisabled(disabled);
	ImGui::SetNextItemWidth(getInputWidth());
	if (ImGui::Button("Create")) {
		clickedCreate = true;
	}
	ImGui::EndDisabled();
}

void nv::editor::FileDialogBase::showCancelButton(bool& cancelled) {
	ImGui::SetNextItemWidth(getInputWidth());
	if (ImGui::Button("Cancel")) {
		cancelled = true;
	}
}

nv::editor::FileDialogBase::FileDialogBase() {
	m_directoryStack.emplace_back(0); //root directory ID is always 0
}

std::optional<nv::editor::FileID> nv::editor::FileDialog::show(const ProjectFileManager& pfm,
	const DirectoryMap& directories,
	const FileMap& files, File::Type filter, bool& cancelled)
{
	bool clickedCreate = false;
	
	nv::detail::ScopeExit exit{ [] { ImGui::EndPopup(); } };

	ImGui::SetNextWindowSize({ 1800.0f, 1800.0f });
	centerNextWindow();
	
	ImGui::OpenPopup(FILESYSTEM_DIALOG_POPUP_NAME); //scope exit cleans up ImGui::ClosePopup
	if (ImGui::BeginPopup(FILESYSTEM_DIALOG_POPUP_NAME)) {
		showFilesAndDirectories(pfm, directories, files, filter);
		showCreateButton(clickedCreate, getClickedFileID() == FileID::None());
		showCancelButton(cancelled);
	}

	if (clickedCreate) {
		return getClickedFileID();
	} else {
		return std::nullopt;
	}
}

void nv::editor::MultipleFileDialog::showSelectedFiles(const FileMap& files) {
	auto deletedFileID = FileID::None();
	for (const auto& fileID : m_currChosenFileIDs) {
		ImGui::PushID(getTemporaryImGuiID());
		if (ImGui::Button("X")) {
			deletedFileID = fileID;
		}
		ImGui::PopID();

		ImGui::SameLine();

		ImGui::PushID(getTemporaryImGuiID());
		ImGui::TextUnformatted(files.at(fileID).getName().c_str());
		ImGui::PopID();
	}
	if (deletedFileID != FileID::None()) {
		m_currChosenFileIDs.erase(deletedFileID);
	}
}

void nv::editor::MultipleFileDialog::showAddButton() {
	auto clickedFileID = getClickedFileID();
	ImGui::BeginDisabled(clickedFileID == FileID::None());
	if (ImGui::Button("Add")) {
		m_currChosenFileIDs.insert(clickedFileID);
	}
	ImGui::EndDisabled();
}

std::optional<FileSet> nv::editor::MultipleFileDialog::show(const ProjectFileManager& pfm,
	const DirectoryMap& directories, const FileMap& files,
	File::Type filter, bool& cancelled)
{
	bool clickedCreate = false;

	nv::detail::ScopeExit exit{ [] { ImGui::EndPopup(); } };

	ImGui::SetNextWindowSize({ 800.0f, 800.0f });
	centerNextWindow();
	
	ImGui::OpenPopup(FILESYSTEM_DIALOG_POPUP_NAME);
	if (ImGui::BeginPopup(FILESYSTEM_DIALOG_POPUP_NAME)) {
		showFilesAndDirectories(pfm, directories, files, filter);
		showCreateButton(clickedCreate, m_currChosenFileIDs.empty());
		showCancelButton(cancelled);
		showAddButton();
		showSelectedFiles(files);
	}

	if (clickedCreate) {
		return m_currChosenFileIDs;
	} else {
		return std::nullopt;
	}
}

struct nv::editor::FileDialogSerializer {
	static constexpr const char* ROOT_DIRECTORY_ID = "Root_Directory_ID";

	template<typename FileDialog>
	static void fromJsonImpl(const nlohmann::json& j, FileDialog& dialog) {
		auto rootDirID = j[ROOT_DIRECTORY_ID].get<DirectoryID>();
		dialog.m_currDirectoryID = rootDirID;
		dialog.m_directoryStack.push_back(rootDirID);
	}
	template<typename FileDialog>
	static void toJsonImpl(nlohmann::json& j, const FileDialog& dialog) {
		j[ROOT_DIRECTORY_ID] = dialog.m_directoryStack.front();
	}
};

void nv::editor::from_json(const nlohmann::json& j, FileDialog& dialog) {
	FileDialogSerializer::fromJsonImpl(j, dialog);
}

void nv::editor::to_json(nlohmann::json& j, const FileDialog& dialog) {
	FileDialogSerializer::toJsonImpl(j, dialog);
}

void nv::editor::from_json(const nlohmann::json& j, MultipleFileDialog& dialog) {
	FileDialogSerializer::fromJsonImpl(j, dialog);
}

void nv::editor::to_json(nlohmann::json& j, const MultipleFileDialog& dialog) {
	FileDialogSerializer::toJsonImpl(j, dialog);
}
