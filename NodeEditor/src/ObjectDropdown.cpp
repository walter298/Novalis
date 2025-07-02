#include <novalis/detail/ScopeExit.h>
#include <novalis/detail/file/File.h>
#include <novalis/detail/serialization/BufferedNodeSerialization.h>

#include "ObjectDropdown.h"
#include "WindowLayout.h"

using namespace nv;
using namespace editor;

constexpr ImVec2 SPRITESHEET_CREATION_WINDOW_SIZE{ 500.0f, 500.0f };

static std::optional<EditedObjectData<BufferedNode>> uploadNode() noexcept {
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
		EditedObjectData<BufferedNode> ret{ nodeJson.get<BufferedNode>() };
		ret.filePath = *filePath;
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
	} catch (std::exception e) {
		std::println("{}", e.what());
		return std::nullopt;
	}
}

static std::optional<std::vector<EditedObjectData<Texture>>> uploadTextures(SDL_Renderer* renderer) noexcept {
	auto texPaths = openMultipleFiles({ { "images", "png" } });
	if (!texPaths) {
		return std::nullopt;
	}

	std::vector<EditedObjectData<Texture>> textures;

	for (const auto& texPath : *texPaths) {
		detail::TexturePtr tex{ renderer, texPath.c_str() };
		if (tex.tex == nullptr) {
			std::println(stderr, "{}", SDL_GetError());
			continue;
		}
		auto& editedTex = textures.emplace_back(std::move(tex));
		editedTex.texPath = texPath;
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

static void inputSpritesheetNum(const char* label, int& num) noexcept {
	auto temp = num;
	ImGui::SetNextItemWidth(getInputWidth());
	ImGui::InputInt(label, &temp);
	if (temp > 0 && temp < 100) {
		num = temp;
	}
}

static SDL_FRect getSpritesheetRect(const detail::TexturePtr& tex) noexcept {
	auto [winX, winY] = ImGui::GetWindowPos();
	auto [winW, winH] = ImGui::GetWindowSize();
	auto [screenX, screenY] = ImGui::GetIO().DisplaySize;

	return SDL_FRect{ 
		(winW / 2.0f) - (static_cast<float>(tex.tex->w) / 2.0f), winY + winH, 
		static_cast<float>(tex.tex->w), static_cast<float>(tex.tex->h) 
	};
}

static void renderLoadedSpritesheet(SDL_Renderer* renderer, detail::TexturePtr& tex, int rowC, int colC) noexcept 
{
	auto originalRenderTarget = SDL_GetRenderTarget(renderer);
	SDL_SetRenderTarget(renderer, nullptr); //render to the window again

	//store the initial draw color
	SDL_Color originalDrawColor{};
	SDL_GetRenderDrawColor(renderer, &originalDrawColor.r, &originalDrawColor.g, &originalDrawColor.b, &originalDrawColor.a);

	
	//render black background behind spritesheet
	auto rect = getSpritesheetRect(tex);
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_RenderFillRect(renderer, &rect);

	//render the texture
	ImVec2 imageSize{ static_cast<float>(tex.tex->w), static_cast<float>(tex.tex->h) };
	ImGui::Image(reinterpret_cast<ImTextureID>(tex.tex), imageSize);
	
	//restore old render information
	SDL_SetRenderDrawColor(renderer, originalDrawColor.r, originalDrawColor.g, originalDrawColor.b, originalDrawColor.a);
	SDL_SetRenderTarget(renderer, originalRenderTarget);
}

void nv::editor::ObjectDropdown::showSpritesheetCreationWindow(SDL_Renderer* renderer, NodeEditor& currTab) {
	ImGui::OpenPopup(SPRITESHEET_CREATION_POPUP_NAME);
	if (ImGui::BeginPopup(SPRITESHEET_CREATION_POPUP_NAME, WINDOW_FLAGS | ImGuiWindowFlags_HorizontalScrollbar)) {
		//input # of rows and columns
		inputSpritesheetNum("rows", m_rowC);
		inputSpritesheetNum("columns", m_colC);

		/*have button to create the spritesheet. OTHERWISE render it. If we do both, we get a null dereference
		because we move from our texture*/
		ImGui::SetNextItemWidth(getInputWidth());
		if (ImGui::Button("Create Spritesheet")) {
			EditedObjectData<Spritesheet> spritesheet{ std::move(m_tex), m_rowC, m_colC };
			spritesheet.texPath = m_texPath;
			currTab.transfer(std::move(spritesheet));
			m_creatingSpritesheet = false;
		} else {
			renderLoadedSpritesheet(renderer, m_tex, m_rowC, m_colC);
		}
		
		ImGui::EndPopup();
	}
}

void nv::editor::ObjectDropdown::uploadSpriteSheet(SDL_Renderer* renderer, NodeEditor& currTab) {
	auto textureRes = uploadTexture(renderer, m_texPath);
	if (!textureRes) {
		m_creatingSpritesheet = false;
		return;
	}

	m_tex = std::move(*textureRes);
	
	m_creatingSpritesheet = true;
}

static void insertNodeFromFile(NodeEditor& currTab) {
	auto node = uploadNode();
	if (node) {
		currTab.transfer(std::move(*node));
	}
}

void nv::editor::ObjectDropdown::show(SDL_Renderer* renderer, NodeTabList& tabs) {
	if (!tabs.currentTab() || tabs.currentTab()->hasNoLayers() || tabs.currentTab()->isBusy()) {
		showDisabledMenu("Object");
		return;
	}

	if (m_creatingSpritesheet) {
		showSpritesheetCreationWindow(renderer, *tabs.currentTab());
	}

	if (ImGui::BeginMenu("Object")) {
		if (ImGui::MenuItem("Create Textures From Images")) {
			createTexturesFromImages(renderer, *tabs.currentTab());
		}
		ImGui::Separator();
		if (ImGui::MenuItem("Upload Sprite Sheet")) {
			uploadSpriteSheet(renderer, *tabs.currentTab());
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
			insertNodeFromFile(*tabs.currentTab());
		}
		ImGui::Separator();
		if (ImGui::MenuItem("Create Object Group")) {
			tabs.currentTab()->createObjectGroup();
		}
		ImGui::EndMenu();
	}
}

bool nv::editor::ObjectDropdown::creatingSpritesheet() const noexcept {
	return m_creatingSpritesheet;
}

