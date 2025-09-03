#include <magic_enum/magic_enum.hpp>
#include <novalis/detail/ScopeExit.h>
#include <novalis/detail/file/File.h>
#include <novalis/detail/serialization/BufferedNodeSerialization.h>

#include "ObjectDropdown.h"
#include "WindowLayout.h"

using namespace nv;
using namespace editor;

namespace {
	enum State {
		CreatingSpritesheetFromMultipleImages,
		CreatingSpritesheetFromSingleImage,
		OpeningSingleSpritesheet,
		OpeningMultispritesheet,
		OpeningTextures,
		UploadingNode,
		None
	};
	State objectDropdownState = None;
	SpritesheetCreator spritesheetCreator;
	MultiSpritesheetCreator multiSpritesheetCreator;

	template<typename DialogHandler, typename Func>
	void handleFileDialog(const ProjectFileManager& pfm, VirtualFilesystem& vfs, DialogHandler fileDialog, File::Type filter,
		Func f) 
	{
		bool cancelled = false;
		auto res = std::invoke(fileDialog, vfs, pfm, filter, cancelled);
		if (cancelled) {
			objectDropdownState = None;
		}
		if (res) {
			f(*res);
		}
	}

	void showSingleImageDialog(const ProjectFileManager& pfm, VirtualFilesystem& vfs, ErrorPopup& errorPopup) {
		auto init = [&](FileID id) {
			try {
				auto texPath = pfm.getSharedAssetPath(id);
				auto instance = nv::getGlobalInstance();
				auto tex = instance->registry.loadTexture(instance->getRenderer(), texPath);
				spritesheetCreator.init(std::move(tex), id);
			} catch (const std::exception& e) {
				errorPopup.add(e.what());
				objectDropdownState = None;
				return;
			}
			objectDropdownState = CreatingSpritesheetFromSingleImage;
		};
		handleFileDialog(pfm, vfs, &VirtualFilesystem::showFileDialog, File::Type::Image, init);
	}
	void openMultipleImagesForSpritesheet(SDL_Renderer* renderer, const ProjectFileManager& pfm, 
		VirtualFilesystem& vfs, ErrorPopup& errorPopup) 
	{
		auto init = [&](const FileSet& imageIDs) {
			try {
				std::vector textures{
					std::from_range, imageIDs | std::views::transform([&](FileID id) {
						auto path = pfm.getSharedAssetPath(id).string();
						return nv::detail::TexturePtr{ renderer, path.c_str() };
					})
				};
				multiSpritesheetCreator.init(renderer, textures, errorPopup);
				objectDropdownState = CreatingSpritesheetFromMultipleImages;
			} catch (const std::exception& e) {
				errorPopup.add(e.what());
			}
		};
		handleFileDialog(pfm, vfs, &VirtualFilesystem::showMultipleFileDialog, File::Type::Image, init);
	}
	void uploadNode(Project& project, ErrorPopup& errorPopup) {
		auto init = [&](FileID childID) {
			auto& currTab = *project.tabManager.getCurrentNodeTab();
			if (project.vfs.createDependency(currTab.getID(), childID)) {
				auto nodePath = project.pfm.getSharedAssetPath(childID);
				auto node = project.tabManager.getNodeTab(project.pfm, project.getCurrentVersion(), 
					childID, errorPopup);
				if (node) {
					ObjectMetadata<BufferedNode> nodeData{ *node->node };
					nodeData.filePath = nodePath.string();
					currTab.transfer(childID, std::move(nodeData));
				}
			} else {
				errorPopup.add("Error: cannot create circular dependency");
			}
			objectDropdownState = None;
		};
		handleFileDialog(project.pfm, project.vfs, &VirtualFilesystem::showFileDialog, File::Type::Node, init);
	}
	void openTextures(SDL_Renderer* renderer, Project& project) {
		auto init = [&](const FileSet& imageIDs) {
			std::vector textures{
				std::from_range, imageIDs | std::views::transform([&](FileID id) {
					auto path = project.pfm.getSharedAssetPath(id).string();
					nv::detail::TexturePtr tex{ renderer, path.c_str() };
					ObjectMetadata<Texture> ret{ std::move(tex) };
					ret.texFile = id;
					ret.texPath = std::move(path);
					return ret;
				})
			};
			project.tabManager.getCurrentNodeTab()->transfer(textures);
			objectDropdownState = None;
		};
		handleFileDialog(project.pfm, project.vfs, &VirtualFilesystem::showMultipleFileDialog, File::Type::Image, init);
	}

	template<typename SpritesheetCreator>
	void createSpritesheet(SDL_Renderer* renderer, Project& project, SpritesheetCreator& spritesheetCreator,
		ErrorPopup& errorPopup) 
	{
		bool cancelled = false;
		auto spritesheetRes = spritesheetCreator.show(renderer, project.pfm, cancelled, errorPopup);
		if (cancelled) {
			objectDropdownState = None;
		}
		if (spritesheetRes) {
			auto& currTab = *project.tabManager.getCurrentNodeTab();
			project.vfs.createDependency(currTab.getID(), spritesheetRes->texFile);
			currTab.transfer(std::move(*spritesheetRes));
			objectDropdownState = None;
		}
	}
}

void nv::editor::showObjectDropdown(SDL_Renderer* renderer, Project& project, ErrorPopup& errorPopup) {
	auto currTab = project.tabManager.getCurrentNodeTab();
	if (!currTab || currTab->hasNoLayers() || currTab->isBusy()) {
		showDisabledMenu("Object");
		return;
	}

	switch (objectDropdownState) {
	case CreatingSpritesheetFromMultipleImages:
		createSpritesheet(renderer, project, multiSpritesheetCreator, errorPopup);
		break;
	case CreatingSpritesheetFromSingleImage:
		createSpritesheet(renderer, project, spritesheetCreator, errorPopup);
		break;
	case OpeningSingleSpritesheet:
		showSingleImageDialog(project.pfm, project.vfs, errorPopup);
		break;
	case OpeningMultispritesheet:
		openMultipleImagesForSpritesheet(renderer, project.pfm, project.vfs, errorPopup);
		break;
	case UploadingNode:
		uploadNode(project, errorPopup);
		break;
	case OpeningTextures:
		openTextures(renderer, project);
		break;
	}

	ImGui::BeginDisabled(isObjectDropdownBusy());

	if (ImGui::BeginMenu("Object")) {
		if (ImGui::MenuItem("Create Textures From Images")) {
			objectDropdownState = OpeningTextures;
		}
		ImGui::Separator();
		if (ImGui::MenuItem("Create Spritesheet From Single Image")) {
			objectDropdownState = OpeningSingleSpritesheet;
		}
		if (ImGui::MenuItem("Create Spritesheet From Multiple Images")) {
			objectDropdownState = OpeningMultispritesheet;
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
			objectDropdownState = UploadingNode;
		}
		ImGui::Separator();
		if (ImGui::MenuItem("Create Object Group")) {
			currTab->createObjectGroup();
		}
		ImGui::EndMenu();
	}

	ImGui::EndDisabled();
}

bool nv::editor::isObjectDropdownBusy() noexcept {
	return objectDropdownState != None;
}
