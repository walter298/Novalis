#pragma once

#include <optional>
#include <vector>

#include "ImageType.h"
#include "EditedObjectData.h"
#include "ErrorPopup.h"

namespace nv {
	namespace editor {
		class ProjectFileManager;

		class MultiSpritesheetCreator {
		private:
			struct Image {
				SDL_Surface* surface = nullptr; //will be concatenated into a single spritesheet
				SDL_Texture* imagePreview = nullptr; //store an extra SDL_Texture* to pass to ImGui::Image
				size_t originalIdx = 0; //index of the image in the original vector of images, used to restore order
			};
			File::Type m_imageType{ File::Type::PNG };
			std::vector<Image> m_images;
			std::string m_enumBuff;
			int m_rowC = 1;
			int m_colC = 1;

			void showImageDropdown();
			void showSpriteTable();
			std::optional<SDL_Surface*> combineSurfaces(ErrorPopup& errorPopup) noexcept;
			std::optional<ObjectMetadata<Spritesheet>> concatenateImagesIntoSpritesheet(SDL_Renderer* renderer,
				ProjectFileManager& pfm, ErrorPopup& errorPopup);
		public:
			void init(SDL_Renderer* renderer,
				const std::vector<nv::detail::TexturePtr>& textures, ErrorPopup& errorPopup);
			std::optional<ObjectMetadata<Spritesheet>> show(SDL_Renderer* renderer, 
				ProjectFileManager& pfm, bool& cancelled, ErrorPopup& errorPopup);
		};
	}
}