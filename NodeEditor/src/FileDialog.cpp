#include <novalis/Instance.h>
#include <novalis/detail/ScopeExit.h>

#include "FileDialog.h"
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

	ImTextureID getFileTex(const File& file) {
		if (static_cast<int>((file.getType() & File::Type::Image)) != 0) {
			auto instance = getGlobalInstance();
			auto tex = instance->registry.loadTexture(instance->getRenderer(), file.getPath().string());
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

void nv::editor::FileDialogBase::showFiles(const FileSet& displayedFiles, const FileMap& fileMap, 
	File::Type filter) 
{
	for (const auto& fileID : displayedFiles) {
		auto& file = fileMap.at(fileID);
		if ((file.getType() & filter) == 0) {
			continue;
		}

		showIcon(getFileTex(file), file.getName(), FILE_ICON_SIZE, fileID == m_currClickedFileID);
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

void nv::editor::FileDialogBase::showFilesAndDirectories(const DirectoryMap& dirMap, const FileMap& fileMap,
	File::Type filter) 
{
	showDirectoryStack(dirMap);
	ImGui::NewLine();

	const auto& directory = dirMap.at(m_currDirectoryID);
	showFiles(directory.files, fileMap, filter);
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

nv::editor::FileDialogBase::FileDialogBase(DirectoryID rootDirID) : m_currDirectoryID{ rootDirID }
{
	m_directoryStack.push_back(rootDirID);
}

std::optional<nv::editor::FileID> nv::editor::FileDialog::show(const DirectoryMap& directories,
	const FileMap& files, File::Type filter, bool& cancelled)
{
	bool clickedCreate = false;
	
	nv::detail::ScopeExit exit{ [] { ImGui::EndPopup(); } };

	ImGui::SetNextWindowSize({ 1800.0f, 1800.0f });
	centerNextWindow();
	
	ImGui::OpenPopup(FILESYSTEM_DIALOG_POPUP_NAME); //scope exit cleans up ImGui::ClosePopup
	if (ImGui::BeginPopup(FILESYSTEM_DIALOG_POPUP_NAME)) {
		showFilesAndDirectories(directories, files, filter);
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

std::optional<FileSet> nv::editor::MultipleFileDialog::show(const DirectoryMap& directories, const FileMap& files,
	File::Type filter, bool& cancelled)
{
	bool clickedCreate = false;

	nv::detail::ScopeExit exit{ [] { ImGui::EndPopup(); } };

	ImGui::SetNextWindowSize({ 800.0f, 800.0f });
	centerNextWindow();
	
	ImGui::OpenPopup(FILESYSTEM_DIALOG_POPUP_NAME);
	if (ImGui::BeginPopup(FILESYSTEM_DIALOG_POPUP_NAME)) {
		showFilesAndDirectories(directories, files, filter);
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