#include "Enum.h"
#include "MultispritesheetCreator.h"
#include "SpritesheetDimensionInput.h"
#include "VirtualFilesystem.h"
#include "WindowLayout.h"

#include <novalis/detail/ScopeExit.h>

void nv::editor::MultiSpritesheetCreator::showImageDropdown() {
	ImGui::SetNextItemWidth(getInputWidth());

	getEnumName(m_enumBuff, m_imageType);
	if (ImGui::BeginCombo("Image Type", m_enumBuff.c_str())) {
		for (const auto& imageType : magic_enum::enum_values<ImageType>()) {
			getEnumName(m_enumBuff, m_imageType);
			if (ImGui::Selectable(m_enumBuff.c_str(), m_imageType == imageType)) {
				m_imageType = imageType;
			}
		}
		ImGui::EndCombo();
	}
}

std::optional<SDL_Surface*> nv::editor::MultiSpritesheetCreator::combineSurfaces(ErrorPopup& errorPopup) noexcept {
	auto verifyDimension = [&](const char* dimName, int dim, float imageLen) -> bool {
		constexpr auto MAX_DIMENSION = 16000;
		auto s = static_cast<float>(dim) * imageLen;
		if (s > MAX_DIMENSION) {
			auto errorMessage = std::format("Error: spritesheet {} ({}) exceeds maximum allowed size of {} pixels.", dimName, s, MAX_DIMENSION);
			errorPopup.add(std::move(errorMessage));
			return false;
		}
		return true;
	};

	if (verifyDimension("width", m_colC, static_cast<float>(m_images.front().surface->w)) &&
		verifyDimension("height", m_rowC, static_cast<float>(m_images.front().surface->h)))
	{
		auto w = m_colC * m_images.front().surface->w;
		auto h = m_rowC * m_images.front().surface->h;
		auto combinedSurface = SDL_CreateSurface(w, h, SDL_PIXELFORMAT_RGBA32);
		if (!combinedSurface) {
			errorPopup.add(std::format("Error: could not create spritesheet surface: {}", SDL_GetError()));
			return std::nullopt;
		}

		for (int i = 0; i < m_rowC; i++) {
			for (int j = 0; j < m_colC; j++) {
				auto surfaceIdx = static_cast<size_t>(i * m_colC + j);
				if (!(surfaceIdx < m_images.size())) {
					return combinedSurface;
				}
				auto& surface = m_images[surfaceIdx].surface;
				SDL_Rect dstRect{ j * surface->w, i * surface->h, surface->w, surface->h };
				SDL_BlitSurface(surface, nullptr, combinedSurface, &dstRect);
			}
		}
	}
	return std::nullopt;
}

void nv::editor::MultiSpritesheetCreator::showSpriteTable() {
	ImVec2 cellSize{
		static_cast<float>(m_images.front().surface->w),
		static_cast<float>(m_images.front().surface->h)
	};
	clampImVec2(cellSize, { 500.0f, 500.0f });

	constexpr auto PAYLOAD_NAME = "PAYLOAD";

	if (ImGui::BeginTable("##SpritesheetGrid", m_colC)) {
		for (int i = 0; i < m_rowC; i++) {
			ImGui::TableNextRow();

			for (int j = 0; j < m_colC; j++) {
				ImGui::TableSetColumnIndex(j);

				auto surfaceIdx = static_cast<size_t>(i * m_colC + j);
				if (!(surfaceIdx < m_images.size())) {
					ImGui::Dummy(cellSize); //fill empty space
					continue;
				}

				ImGui::Image(reinterpret_cast<ImTextureID>(m_images[surfaceIdx].imagePreview), cellSize);

				if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)) {
					ImGui::SetDragDropPayload(PAYLOAD_NAME, &surfaceIdx, sizeof(size_t));
					ImGui::Image(reinterpret_cast<ImTextureID>(m_images[surfaceIdx].imagePreview), cellSize);
					ImGui::EndDragDropSource();
				}

				if (ImGui::BeginDragDropTarget()) {
					auto payload = ImGui::AcceptDragDropPayload(PAYLOAD_NAME);
					if (payload) {
						auto droppedSurfaceIdx = *(reinterpret_cast<size_t*>(payload->Data));
						std::swap(m_images[surfaceIdx], m_images[droppedSurfaceIdx]);
					}
					ImGui::EndDragDropTarget();
				}
			}
		}
		ImGui::EndTable();
	}
}

std::optional<nv::editor::ObjectMetadata<nv::Spritesheet>> nv::editor::MultiSpritesheetCreator::concatenateImagesIntoSpritesheet(
	SDL_Renderer* renderer, VirtualFilesystem& vfs, ErrorPopup& errorPopup)
{
	assert(m_images.size() > 1);
	if (static_cast<size_t>(m_rowC * m_colC) < m_images.size()) {
		errorPopup.add("Error: not enough rows and columns to fit all m_images in the spritesheet.");
		return std::nullopt;
	}

	//verify that spritesheet won't exceed SDL's maximum texture size of 16000x16000
	auto verifyDimension = [&](const char* dimName, int dim, float imageLen) -> bool {
		constexpr auto MAX_DIMENSION = 16000;
		auto s = static_cast<float>(dim) * imageLen;
		if (s > MAX_DIMENSION) {
			auto errorMessage = std::format("Error: spritesheet {} ({}) exceeds maximum allowed size of {} pixels.", dimName, s, MAX_DIMENSION);
			errorPopup.add(std::move(errorMessage));
			return false;
		}
		return true;
	};

	auto combinedSurface = combineSurfaces(errorPopup);
	if (!combinedSurface) {
		return std::nullopt;
	}
	nv::detail::ScopeExit cleanup{ [&] { SDL_DestroySurface(*combinedSurface); } };

	ObjectMetadata<Spritesheet> spritesheet{
		nv::detail::TexturePtr{ SDL_CreateTextureFromSurface(renderer, *combinedSurface) }, m_rowC, m_colC
	};
	spritesheet.texFile = vfs.saveImage(*combinedSurface, m_imageType);

	//reset spritesheet state
	for (const auto& [surface, imagePreview, idx] : m_images) {
		SDL_DestroySurface(surface);
		SDL_DestroyTexture(imagePreview);
	}
	m_images.clear();

	return spritesheet;
}

void nv::editor::MultiSpritesheetCreator::init(SDL_Renderer* renderer, 
	const std::vector<nv::detail::TexturePtr>& textures, ErrorPopup& errorPopup)
{
	SDL_RenderClear(renderer);

	auto originalRenderTarget = SDL_GetRenderTarget(renderer);
	auto surfaceCreator = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_TARGET, 10000, 10000);
	SDL_SetRenderTarget(renderer, surfaceCreator);
	
	std::vector<Image> surfaces;
	for (const auto& [idx, tex] : std::views::enumerate(textures)) {
		SDL_RenderTexture(renderer, tex.tex, nullptr, nullptr);
		SDL_Rect texRect{ 0, 0, tex.tex->w, tex.tex->h };
		auto surface = SDL_RenderReadPixels(renderer, &texRect);
		auto imagePreview = SDL_CreateTextureFromSurface(renderer, surface);
		surfaces.emplace_back(surface, imagePreview, static_cast<size_t>(idx));

		SDL_RenderClear(renderer);
	}
}

std::optional<nv::editor::ObjectMetadata<nv::Spritesheet>> nv::editor::MultiSpritesheetCreator::show(
	SDL_Renderer* renderer, VirtualFilesystem& vfs, bool& cancelled, ErrorPopup& errorPopup)
{
	ImGui::OpenPopup(SPRITESHEET_CREATION_POPUP_NAME, ImGuiWindowFlags_HorizontalScrollbar);
	if (ImGui::BeginPopup(SPRITESHEET_CREATION_POPUP_NAME)) {
		nv::detail::ScopeExit exit{ [] { ImGui::EndPopup(); } };

		inputSpritesheetDimensions(m_rowC, m_colC);
		showImageDropdown();

		//sprite original re-ordering
		ImGui::SetNextItemWidth(getInputWidth());
		if (ImGui::Button("Reset Order")) {
			std::ranges::sort(m_images, [](const auto& a, const auto& b) {
				return a.originalIdx < b.originalIdx;
			});
		}

		ImGui::SetNextItemWidth(getInputWidth());
		if (ImGui::Button("Create Spritesheet")) {
			return concatenateImagesIntoSpritesheet(renderer, vfs, errorPopup);
		}

		ImGui::SetNextItemWidth(getInputWidth());
		if (ImGui::Button("Cancel")) {
			cancelled = true;
			return std::nullopt; 
		}

		showSpriteTable();
	}

	return std::nullopt;
}