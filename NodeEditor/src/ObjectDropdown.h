#pragma once

#include <variant>

#include "NodeTabList.h"
#include "Project.h"
#include "SpritesheetCreator.h"
#include "MultiSpritesheetCreator.h"

namespace nv {
	namespace editor {
		class ObjectDropdown {
		private:
			SpritesheetCreator m_spritesheetCreator;
			MultiSpritesheetCreator m_multiSpritesheetCreator;
			enum State {
				CreatingSpritesheetFromMultipleImages,
				CreatingSpritesheetFromSingleImage,
				OpeningSingleImageDialog,
				OpeningMultipleImagesDialog,
				None
			} m_state = None;
			void openMultiImageDialog(SDL_Renderer* renderer, VirtualFilesystem& vfs, ErrorPopup& errorPopup);
			void openImageDialog(VirtualFilesystem& vfs);
		public:
			void show(SDL_Renderer* renderer, Project& project, ErrorPopup& errorPopup);
			bool isBusy() const noexcept;
		};
	}
}