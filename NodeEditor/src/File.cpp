#include <novalis/detail/file/File.h>
#include <novalis/detail/reflection/ClassMemberFilter.h>
#include <novalis/detail/serialization/AutoSerialization.h>

#include "File.h"
#include "ImGuiID.h"
#include "NovalisRoot.h"

namespace {
	nv::detail::TexturePtr openFolderTex;
	nv::detail::TexturePtr closedFolderTex;
	nv::detail::TexturePtr nodeFileIconTex;
	nv::detail::TexturePtr pngFileIconTex;
	nv::detail::TexturePtr avifFileIconTex;
	nv::detail::TexturePtr jpgFileIconTex;
	nv::detail::TexturePtr bmpFileIconTex;

	ImTextureID parseIcon(const std::filesystem::path& path) {
		static boost::unordered_flat_map<std::string, SDL_Texture*> fileIcons{
			{ ".json", nodeFileIconTex.tex },
			{ ".avif", avifFileIconTex.tex },
			{ ".png", pngFileIconTex.tex },
			{ ".jpg", jpgFileIconTex.tex },
			{ ".bmp", bmpFileIconTex.tex }
		};
		auto fileExtension = path.extension().string();
		assert(!fileExtension.empty());
		for (auto& chr : fileExtension) {
			chr = tolower(chr);
		}
		return reinterpret_cast<ImTextureID>(fileIcons.at(fileExtension));
	}
}

void nv::editor::loadFileIcons(SDL_Renderer* renderer) {
#if _DEBUG
	static bool loaded = false;
	assert(!loaded);
#endif
	auto folderImageDir = getNovalisRoot() / "NodeEditor/novalis_assets/file_explorer_images";

	auto loadTexture = [&](std::string relativeImagePath) -> nv::detail::TexturePtr {
		auto path = folderImageDir / relativeImagePath;
		return { renderer, path.string().c_str() };
	};
	openFolderTex = loadTexture("open_folder.png");
	closedFolderTex = loadTexture("closed_folder.png");
	nodeFileIconTex = loadTexture("node_file_icon.png");
	pngFileIconTex = loadTexture("png_file_icon.png");
	jpgFileIconTex = loadTexture("jpg_file_icon.png");
	bmpFileIconTex = loadTexture("bmp_file_icon.png");
	avifFileIconTex = loadTexture("avif_file_icon.png");
}

void nv::editor::destroyFileIcons(SDL_Renderer* renderer) {
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

void nv::editor::to_json(nlohmann::json& j, const File& file) {
	j = file.__makeMemberTuple();
}

void nv::editor::from_json(const nlohmann::json& j, File& file) {
	auto serializedData = j.get<File::__Tuple>();
	auto members = file.__makeMemberTuple();
	nv::detail::forEachDataMember([](auto& member, auto& serializedMember) {
		member = serializedMember;
		return nv::detail::STAY_IN_LOOP;
	}, members, serializedData);
	file.m_icon = parseIcon(file.m_realPath);
}

ImTextureID nv::editor::getFolderIcon() noexcept {
	return reinterpret_cast<ImTextureID>(closedFolderTex.tex);
}

nv::editor::File::File(std::filesystem::path realPath, NameManager& parentNameManager, std::string name, File::Type type)
	: m_realPath{ std::move(realPath) }, m_name{ std::move(name) }, m_type{ type }
{
	m_icon = parseIcon(m_realPath);
}

nv::editor::File::File(std::filesystem::path realPath, NameManager& parentNameManager, File::Type type)
	: File{ std::move(realPath), parentNameManager, "", type }
{
}

void nv::editor::File::show() noexcept {
	ImGui::PushID(m_imguiID);
	ImGui::Image(m_icon, FILE_ICON_SIZE);
	ImGui::SameLine();
	ImGui::TextUnformatted(m_name.c_str());
	ImGui::PopID();
}

void nv::editor::File::show(NameManager& dirNameManager, bool& finishedInput) noexcept {
	ImGui::PushID(m_imguiID);
	ImGui::Image(m_icon, FILE_ICON_SIZE);
	ImGui::SameLine();
	finishedInput = dirNameManager.inputName("", m_name);
	ImGui::PopID();
}

void nv::editor::File::makeNameUnique(NameManager& parentNameManager) {
	parentNameManager.makeExistingNameUnique(m_name);
}

const std::string& nv::editor::File::getName() const noexcept {
	return m_name;
}

const std::filesystem::path& nv::editor::File::getPath() const noexcept {
	return m_realPath;
}

nv::editor::File::Type nv::editor::File::getType() const noexcept {
	return m_type;
}

ImTextureID nv::editor::File::getIcon() const noexcept {
	return m_icon;
}

nv::editor::Directory nv::editor::Directory::makeRoot() {
	Directory ret;
	ret.m_name = "root";
	return ret;
}

nv::editor::Directory::Directory(NameManager& parentNameManager, std::string name) : m_name{ std::move(name) }
{
	parentNameManager.makeExistingNameUnique(m_name);
}

nv::editor::Directory::Directory(NameManager& parentNameManager)
	: Directory{ parentNameManager, "" }
{
}

void nv::editor::Directory::inputName(NameManager& parentNameManager, bool& finishedRenaming) {
	finishedRenaming = parentNameManager.inputName("", m_name);
}

void nv::editor::Directory::makeNameUnique(NameManager& parentNameManager) {
	parentNameManager.makeExistingNameUnique(m_name);
}

void nv::editor::Directory::showIcon() const noexcept {
	ImGui::PushID(getTemporaryImGuiID());
	auto& folderTex = open ? openFolderTex : closedFolderTex;
	ImGui::Image(reinterpret_cast<ImTextureID>(folderTex.tex), DIRECTORY_ICON_SIZE);
	ImGui::PopID();
}

const std::string& nv::editor::Directory::getName() const noexcept {
	return m_name;
}

int nv::editor::Directory::getImGuiID() {
	return m_imguiID;
}