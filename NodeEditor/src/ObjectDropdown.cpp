#include <magic_enum/magic_enum.hpp>
#include <novalis/detail/ScopeExit.h>
#include <novalis/detail/file/File.h>
#include <novalis/detail/serialization/BufferedNodeSerialization.h>

#include "ObjectDropdown.h"
#include "WindowLayout.h"

using namespace nv;
using namespace editor;

constexpr ImVec2 SPRITESHEET_CREATION_WINDOW_SIZE{ 500.0f, 500.0f };

static std::optional<ObjectMetadata<BufferedNode>> uploadNode() noexcept {
	auto filePath = openFile({ { "node", "nv_node" } });
	if (!filePath) {
		return std::nullopt;
	}
	std::ifstream file{ filePath->c_str() };
	if (!file.is_open()) {
		std::println(stderr, "Error: could not open {}", *filePath);
		return std::nullopt;
	}
	try {
		auto nodeJson = json::parse(file);
		ObjectMetadata<BufferedNode> ret{ nodeJson.get<BufferedNode>() };
		ret.obj.resetWorld();
		return ret;
	} catch (const std::exception& e) {
		std::println(stderr, "{}", e.what());
		return std::nullopt;
	}
}

static std::optional<nv::detail::TexturePtr> uploadTexture(SDL_Renderer* renderer, std::string& texPath) noexcept {
	auto texPathRes = openFile({ {"images", "png" } });
	if (!texPathRes) {
		return std::nullopt;
	}
	try {
		nv::detail::TexturePtr ret{ renderer, texPathRes->c_str() };
		texPath = std::move(*texPathRes);
		return ret;
	} catch (const std::exception& e) {
		std::println("{}", e.what());
		return std::nullopt;
	}
}

static std::optional<std::vector<ObjectMetadata<Texture>>> uploadTextures(SDL_Renderer* renderer) noexcept {
	auto texPaths = openMultipleFiles({ { "images", "png" } });
	if (!texPaths) {
		return std::nullopt;
	}

	std::vector<ObjectMetadata<Texture>> textures;

	for (const auto& texPath : *texPaths) {
		nv::detail::TexturePtr tex{ renderer, texPath.c_str() };
		if (tex.tex == nullptr) {
			std::println(stderr, "{}", SDL_GetError());
			continue;
		}
		auto& editedTex = textures.emplace_back(std::move(tex));
	}

	return textures;
}

static bool createTexturesFromImages(SDL_Renderer* renderer, NodeEditor& currTab) {
	auto texturesRes = uploadTextures(renderer);
	if (!texturesRes) {
		return false;
	}
	currTab.transfer(*texturesRes);
	return true;
}

static void insertNodeFromFile(NodeEditor& currTab) {
	auto node = uploadNode();
	if (node) {
		currTab.transfer(std::move(*node));
	}
}

void nv::editor::ObjectDropdown::openImageDialog(VirtualFilesystem& vfs) {
	bool cancelled = false;
	auto imageFileIDRes = vfs.showFileDialog(File::Type::Image, cancelled);
	if (cancelled) {
		m_state = None;
	}
	if (imageFileIDRes) {
		m_spritesheetCreator.init(vfs.getTexture(*imageFileIDRes), *imageFileIDRes);
		m_state = CreatingSpritesheetFromSingleImage;
	}
}

void nv::editor::ObjectDropdown::show(SDL_Renderer* renderer, Project& project, ErrorPopup& errorPopup) {
	auto currTab = project.tabManager.getCurrentNodeTab();
	if (!currTab || currTab->hasNoLayers() || currTab->isBusy()) {
		showDisabledMenu("Object");
		return;
	}

	auto makeSpritesheet = [&, this](auto& spritesheetCreator) {
		bool cancelled = false;
		auto spritesheetRes = spritesheetCreator.show(renderer, project.vfs, cancelled, errorPopup);
		if (cancelled) {
			m_state = None;
		}
		if (spritesheetRes) {
			auto& currTab = *project.tabManager.getCurrentNodeTab();
			project.vfs.createDependency(currTab.getID(), spritesheetRes->texFile);
			currTab.transfer(std::move(*spritesheetRes));
			m_state = None;
		}
	};
	auto handleImageIDs = [&, this](std::invocable<FileSet> auto func) {
		bool cancelled = false;
		auto images = project.vfs.showMultipleFileDialog(File::Type::Image, cancelled);
		if (cancelled) {
			m_state = None;
		}
		if (images) {
			func(*images);
		}
	};

	switch (m_state) {
	case CreatingSpritesheetFromMultipleImages:
		makeSpritesheet(m_multiSpritesheetCreator);
		break;
	case CreatingSpritesheetFromSingleImage:
		makeSpritesheet(m_spritesheetCreator);
		break;
	case OpeningSingleSpritesheet:
		openImageDialog(project.vfs);
		break;
	case OpeningMultispritesheet:
		handleImageIDs([&, this](const FileSet& imageIDs) {
			std::vector textures{
				std::from_range, imageIDs | std::views::transform([&](FileID id) {
					auto path = project.vfs.getFilePath(id).string();
					return nv::detail::TexturePtr{ renderer, path.c_str() };
				})
			};
			m_multiSpritesheetCreator.init(renderer, textures, errorPopup);
			m_state = CreatingSpritesheetFromMultipleImages;
		});
		break;
	case OpeningTextures:
		handleImageIDs([&, this](const FileSet& imageIDs) {
			std::vector textures{
				std::from_range, imageIDs | std::views::transform([&](FileID id) {
					auto path = project.vfs.getFilePath(id).string();
					nv::detail::TexturePtr tex{ renderer, path.c_str() };
					ObjectMetadata<Texture> ret{ std::move(tex) };
					ret.texFile = id;
					ret.texPath = project.vfs.getFilePath(id).string();
					return ret;
				})
			};
			project.tabManager.getCurrentNodeTab()->transfer(textures);
			m_state = None;
		});
		break;
	};

	ImGui::BeginDisabled(isBusy());

	if (ImGui::BeginMenu("Object")) {
		if (ImGui::MenuItem("Create Textures From Images")) {
			m_state = OpeningTextures;
		}
		ImGui::Separator();
		if (ImGui::MenuItem("Create Spritesheet From Single Image")) {
			m_state = OpeningSingleSpritesheet;
		}
		if (ImGui::MenuItem("Create Spritesheet From Multiple Images")) {
			m_state = OpeningMultispritesheet;
		}
		ImGui::Separator();
		if (ImGui::MenuItem("Create Text")) {
			//todo
		}
		ImGui::Separator();
		if (ImGui::MenuItem("Create Polygon")) {
			//todo
		}
		ImGui::Separator();
		if (ImGui::MenuItem("Upload Node(s)")) {
			insertNodeFromFile(*currTab);
		}
		ImGui::Separator();
		if (ImGui::MenuItem("Create Object Group")) {
			currTab->createObjectGroup();
		}
		ImGui::EndMenu();
	}

	ImGui::EndDisabled();
}

bool nv::editor::ObjectDropdown::isBusy() const noexcept {
	return m_state != None;
}