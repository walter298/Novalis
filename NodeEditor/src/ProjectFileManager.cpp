#include <algorithm>
#include <chrono>
#include <format>
#include <fstream>
#include <magic_enum/magic_enum.hpp>
#include <novalis/detail/ScopeExit.h>
#include <novalis/detail/file/File.h>

#include "ErrorPopup.h"
#include "ProjectFileManager.h"
#include "WindowLayout.h"

std::filesystem::path nv::editor::ProjectFileManager::makeSharedAssetPath(FileID fileID, 
	File::Type type) const
{
	auto enumName = magic_enum::enum_name(type);
	std::string enumStr{ enumName.data(), enumName.size() };
	return m_assetDirPath / (std::to_string(fileID.get()) + "." + enumStr);
}

std::filesystem::path nv::editor::ProjectFileManager::getNodeDirectoryPath(size_t projectIndex) const {
	return m_versionDirPaths[projectIndex] / "nodes";
}

nv::editor::ProjectFileManager::ProjectFileManager(std::filesystem::path rootDirPath)
	: m_rootDirPath{ rootDirPath }
{
	auto internalDirPath = m_rootDirPath / "internal";
	std::filesystem::create_directory(internalDirPath);

	//create version repository
	m_versionHistoryDirPath = internalDirPath / "versions";
	std::filesystem::create_directory(m_versionHistoryDirPath);

	//create asset directory
	m_assetDirPath = internalDirPath / "shared_assets";
	std::filesystem::create_directory(m_assetDirPath);
}

nv::editor::FileID nv::editor::ProjectFileManager::createImage(SDL_Surface* surface, File::Type imageType) {
	FileID id;
	auto path = makeSharedAssetPath(id, imageType);
	auto pathStr = path.string();

	switch (imageType) {
	case File::Type::PNG:
		IMG_SavePNG(surface, pathStr.c_str());
		break;
	case File::Type::JPG:
		IMG_SaveJPG(surface, pathStr.c_str(), 100);
		break;
	case File::Type::BMP:
		SDL_SaveBMP(surface, pathStr.c_str());
		break;
	case File::Type::AVIF:
		IMG_SaveAVIF(surface, pathStr.c_str(), 100);
		break;
	}

	return id;
}

std::optional<size_t> nv::editor::ProjectFileManager::showVersionSelector(size_t currProjectIndex, bool& cancelled) const {
	assert(!cancelled);

	ImGui::SetNextWindowSize({ 800.0f, 800.0f });
	centerNextWindow();
	ImGui::OpenPopup("Load Project Version");

	if (ImGui::BeginPopup("Load Project Version", DEFAULT_WINDOW_FLAGS)) {
		nv::detail::ScopeExit endPopup{ []() { ImGui::EndPopup(); } }; //popup cleanup

		for (size_t i = 0; i < m_versionDirPaths.size(); i++) {
			const auto& versionDirPath = m_versionDirPaths[i];
			auto lastWriteTime = std::filesystem::last_write_time(m_versionDirPaths[i]);
			auto dateStr = std::format("{:%m/%d/%Y %I:%M:%S %p}", lastWriteTime);

			ImGui::PushID(getTemporaryImGuiID());
			nv::detail::ScopeExit popID{ []() { ImGui::PopID(); } }; //ID cleanup
			ImGui::SetNextItemWidth(getInputWidth());
			ImGui::BeginDisabled(i == currProjectIndex);
			nv::detail::ScopeExit endDisabled{ []() { ImGui::EndDisabled(); } }; //disabled cleanup
			if (ImGui::Button(dateStr.c_str())) {
				return i;
			}
		}
		ImGui::SetNextItemWidth(getInputWidth());
		if (ImGui::Button("Cancel")) {
			cancelled = true;
		}
	} 
	return std::nullopt;
}

void nv::editor::ProjectFileManager::updateNodeFile(size_t projectIndex, FileID fileID, 
	const std::string& data) const
{
	auto filePath = getNodePath(projectIndex, fileID);
	std::ofstream file{ filePath };
	assert(file.is_open());
	file << data;
}

void nv::editor::ProjectFileManager::makeSharedAsset(FileID fileID, File::Type type, const std::string& data) {
	auto path = makeSharedAssetPath(fileID, type);
	std::ofstream file{ path, std::ios::binary };
	assert(file.is_open());
	file << data;
	m_sharedAssetPathMap.emplace(fileID, std::move(path));
}

void nv::editor::ProjectFileManager::uploadSharedResource(FileID fileID, 
	const std::filesystem::path& path)
{
	auto fileExtension = path.extension().string();
	auto newPath = m_assetDirPath / (std::to_string(fileID.get()) + fileExtension);

	//there may be pre-existing files with the same ID created in a previous session but not saved
	std::filesystem::copy_file(path, newPath, std::filesystem::copy_options::overwrite_existing);
	m_sharedAssetPathMap.emplace(fileID, std::move(newPath));
}

size_t nv::editor::ProjectFileManager::addProjectVersion() {
	auto versionDirPath = m_versionHistoryDirPath / std::to_string(m_versionDirPaths.size());
	
	std::filesystem::create_directory(versionDirPath);
	m_versionDirPaths.push_back(std::move(versionDirPath));
	std::filesystem::create_directory(getNodeDirectoryPath(m_versionDirPaths.size() - 1));

	return m_versionDirPaths.size() - 1;
}

size_t nv::editor::ProjectFileManager::forkProjectVersion(size_t currProjectVersion) {
	auto newProjectVersion = addProjectVersion();
	std::filesystem::copy(m_versionDirPaths[currProjectVersion], m_versionDirPaths[newProjectVersion], 
		std::filesystem::copy_options::recursive);
	return newProjectVersion;
}

size_t nv::editor::ProjectFileManager::getCurrentVersionCount() const noexcept {
	return m_versionDirPaths.size();
}

std::filesystem::path nv::editor::ProjectFileManager::getFilesystemJSONPath(size_t projectIndex) const {
	return m_versionDirPaths[projectIndex] / "virtual_filesystem.json";
}

std::filesystem::path nv::editor::ProjectFileManager::getNodePath(size_t projectIndex, FileID fileID) const
{
	return getNodeDirectoryPath(projectIndex) / (std::to_string(fileID.get()) + ".json");
}

const std::filesystem::path& nv::editor::ProjectFileManager::getSharedAssetPath(FileID fileID) const {
	return m_sharedAssetPathMap.at(fileID);
}

std::filesystem::path nv::editor::ProjectFileManager::getGlobalProjectFilePath() const {
	return m_rootDirPath / "project.json";
}
