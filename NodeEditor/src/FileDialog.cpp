#include <novalis/Instance.h>
#include <novalis/detail/ScopeExit.h>

#include "FileDialog.h"
#include "WindowLayout.h"

//for static methods
using namespace nv;
using namespace editor;

namespace {
	constexpr ImVec4 DIALOG_BACKGROUND_COLOR{ 0.58f, 0.58f, 0.58f, 1.0f };
	
	//returns whether the file is selected
	bool showFile(const File& file, bool isSelected) {
		bool clicked = false;

		constexpr ImVec2 IMAGE_SIZE{ 350.0f, 350.0f };

		ImGui::InvisibleButton("file", IMAGE_SIZE);
		auto drawList = ImGui::GetWindowDrawList();

		auto buttonMin = ImGui::GetItemRectMin();
		auto buttonMax = ImGui::GetItemRectMax();

		if (ImGui::IsItemHovered() || isSelected) {
			drawList->AddRectFilled(buttonMin, buttonMax, IM_COL32(0, 210, 0, 64));
		}
		if (ImGui::IsItemClicked()) {
			clicked = true;
		}
		
		if (static_cast<int>((file.type & File::Type::Image)) != 0) {
			auto instance = getGlobalInstance();
			auto tex = instance->registry.loadTexture(instance->getRenderer(), file.realPath.string());
			drawList->AddImage(reinterpret_cast<ImTextureID>(tex.tex), buttonMin, buttonMax);
		} else {
			drawList->AddImage(file.icon, buttonMin, buttonMax);
		}
	
		ImGui::TextUnformatted(file.name.c_str());

		return clicked;
	}
}

void nv::editor::FileDialogBase::showFilesAndDirectories(const DirectoryMap& directories, const FileMap& files, 
	File::Type filter) 
{
	const auto& directory = directories.at(m_currDialogDirectoryID);
	const auto& displayedFiles = directory.files;

	const auto [popupW, popupH] = ImGui::GetItemRectSize();
	const auto maxFilesToDisplayInRow = static_cast<int>(popupW / FILE_ICON_SIZE.x);
	int rowFileDisplayCount = 0;
	if (rowFileDisplayCount == 0) { //prevent user from completely skinnifying the display
		rowFileDisplayCount = 1;
	}

	for (const auto& fileID : displayedFiles) {
		auto& file = files.at(fileID);
		if ((file.type & filter) == 0) {
			continue;
		}

		if (showFile(file, fileID == m_currClickedFileDialogID)) {
			m_currClickedFileDialogID = fileID;
		}
		handleFile(fileID);

		if (rowFileDisplayCount + 1 == maxFilesToDisplayInRow) {
			rowFileDisplayCount = 0;
			ImGui::TableNextRow();
		} else {
			rowFileDisplayCount++;
			ImGui::TableNextColumn();
		}
	}
}

void nv::editor::FileDialogBase::showCreateButton(bool& clickedCreate) {
	ImGui::BeginDisabled(m_currClickedFileDialogID == FileID::None());
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
		resetState();
	}
}

void nv::editor::FileDialog::handleFile(FileID fileID) {
	if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
		m_currClickedFileDialogID = fileID;
	}
}

void nv::editor::FileDialog::resetState() {
	m_currClickedFileDialogID = FileID::None();
}

std::optional<nv::editor::FileID> nv::editor::FileDialog::show(const DirectoryMap& directories,
	const FileMap& files, File::Type filter, bool& cancelled)
{
	bool clickedCreate = false;
	
	nv::detail::ScopeExit exit{
		[] { 
			ImGui::EndPopup(); 
			//ImGui::PopStyleColor();
		}
	};

	ImGui::SetNextWindowSize({ 1800.0f, 1800.0f });
	centerNextWindow();
	//ImGui::PushStyleColor(ImGuiCol_PopupBg, DIALOG_BACKGROUND_COLOR);

	ImGui::OpenPopup(FILESYSTEM_DIALOG_POPUP_NAME);
	if (ImGui::BeginPopup(FILESYSTEM_DIALOG_POPUP_NAME)) {
		showFilesAndDirectories(directories, files, filter);
		showCreateButton(clickedCreate);
		showCancelButton(cancelled);
	}

	if (clickedCreate) {
		return m_currClickedFileDialogID;
	} else {
		return std::nullopt;
	}
}

void nv::editor::MultipleFileDialog::updateSelectionRect() noexcept {
	if (ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
		m_setSelection = false;

		auto [mx, my] = ImGui::GetMousePos();
		Point mousePos{ mx, my };

		if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
			m_selectionRect.pos = mousePos;
		} else {
			m_selectionRect.size = mousePos - m_selectionRect.pos;
		}
	} else if (ImGui::IsMouseReleased(ImGuiMouseButton_Left)) {
		m_setSelection = true;
	}
}

void nv::editor::MultipleFileDialog::resetState() {
	m_currClickedFileDialogID = FileID::None();
	m_currChosenFileIDs.clear();
}

void nv::editor::MultipleFileDialog::handleFile(FileID fileID) {
	auto [itemX, itemY] = ImGui::GetItemRectMin();
	auto [itemW, itemH] = ImGui::GetItemRectSize();
	SDL_FRect itemRect{ itemX, itemY, itemW, itemH };
	auto selectionRect = m_selectionRect.sdlRect();

	if (SDL_HasRectIntersectionFloat(&itemRect, &selectionRect)) {
		m_selectedFileIDs.insert(fileID);
	}
}

void nv::editor::MultipleFileDialog::showAddButton() {
	ImGui::BeginDisabled(m_currClickedFileDialogID == FileID::None());

	ImGui::EndDisabled();
}

void nv::editor::MultipleFileDialog::showCurrentlyChosenFileIDs(const FileMap& files) {
	for (const auto& fileID : m_currChosenFileIDs) {
		if (ImGui::Button("X")) {
			m_currChosenFileIDs.erase(fileID);
			break;
		}
		auto& file = files.at(fileID);
		ImGui::BulletText(file.name.c_str());
	}
}

std::optional<FileSet> nv::editor::MultipleFileDialog::show(const DirectoryMap& directories, const FileMap& files,
	File::Type filter, bool& cancelled)
{
	bool clickedCreate = false;

	nv::detail::ScopeExit exit{
		[] {
			ImGui::EndPopup();
			ImGui::PopStyleColor();
		}
	};

	ImGui::SetNextWindowSize({ 800.0f, 800.0f });
	centerNextWindow();
	ImGui::PushStyleColor(ImGuiCol_PopupBg, DIALOG_BACKGROUND_COLOR);

	ImGui::OpenPopup(FILESYSTEM_DIALOG_POPUP_NAME);
	if (ImGui::BeginPopup(FILESYSTEM_DIALOG_POPUP_NAME)) {
		showFilesAndDirectories(directories, files, filter);
		showCreateButton(clickedCreate);
		showCancelButton(cancelled);
	}

	if (clickedCreate) {
		return m_currChosenFileIDs;
	} else {
		return std::nullopt;
	}
}
