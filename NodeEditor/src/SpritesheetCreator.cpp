#include <magic_enum/magic_enum.hpp>
#include <novalis/detail/ScopeExit.h>
#include <novalis/detail/file/File.h>

#include "Enum.h"
#include "SpritesheetCreator.h"
#include "SpritesheetDimensionInput.h"
#include "VirtualFilesystem.h"
#include "WindowLayout.h"

//for static methods
using namespace nv;
using namespace editor;

static SDL_FRect getSpritesheetRect(const nv::detail::TexturePtr& tex) noexcept {
	auto [winX, winY] = ImGui::GetWindowPos();
	auto [winW, winH] = ImGui::GetWindowSize();
	auto [screenX, screenY] = ImGui::GetIO().DisplaySize;

	return SDL_FRect{
		(winW / 2.0f) - (static_cast<float>(tex.tex->w) / 2.0f), winY + winH,
		static_cast<float>(tex.tex->w), static_cast<float>(tex.tex->h)
	};
}

static void renderLoadedSpritesheet(SDL_Renderer* renderer, nv::detail::TexturePtr& tex, int rowC, int colC) noexcept
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

void nv::editor::SpritesheetCreator::init(nv::detail::TexturePtr tex, FileID imageFileID) {
	m_tex = tex;
	m_fileID = imageFileID;
}

std::optional<EditedObjectData<Spritesheet>> nv::editor::SpritesheetCreator::show(SDL_Renderer* renderer,
	VirtualFilesystem& vfs, bool& cancelled, ErrorPopup& errorPopup) 
{
	assert(m_tex.tex);

	ImGui::OpenPopup(SPRITESHEET_CREATION_POPUP_NAME, ImGuiWindowFlags_HorizontalScrollbar);
	if (ImGui::BeginPopup(SPRITESHEET_CREATION_POPUP_NAME)) {
		nv::detail::ScopeExit exit{ [] { ImGui::EndPopup(); } };

		inputSpritesheetDimensions(m_rowC, m_colC);

		/*have button to create the spritesheet. OTHERWISE render it. If we do both, we get a null dereference
		because we move from our texture*/
		ImGui::SetNextItemWidth(getInputWidth());
		if (ImGui::Button("Create Spritesheet")) {
			EditedObjectData<Spritesheet> spritesheet{ std::move(m_tex), m_rowC, m_colC };
			spritesheet.texFile = m_fileID;
			return spritesheet;
		}
		ImGui::SetNextItemWidth(getInputWidth());
		if (ImGui::Button("Cancel")) {
			cancelled = true;
		}
		renderLoadedSpritesheet(renderer, m_tex, m_rowC, m_colC);
	}
	return std::nullopt;
}