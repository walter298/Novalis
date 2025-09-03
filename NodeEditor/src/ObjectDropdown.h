#pragma once

#include <variant>

#include "Project.h"
#include "SpritesheetCreator.h"
#include "MultiSpritesheetCreator.h"

namespace nv {
	namespace editor {
		/*class ObjectDropdown {
		private:
			SpritesheetCreator m_spritesheetCreator;
			MultiSpritesheetCreator m_multiSpritesheetCreator;
			enum State {
				CreatingSpritesheetFromMultipleImages,
				CreatingSpritesheetFromSingleImage,
				OpeningSingleSpritesheet,
				OpeningMultispritesheet,
				OpeningTextures,
				UploadingNode,
				None
			} m_state = None;
			void openImageDialog(VirtualFilesystem& vfs);
		public:
			void show(SDL_Renderer* renderer, Project& project, ErrorPopup& errorPopup);
			bool isBusy() const noexcept;
		};*/

		void showObjectDropdown(SDL_Renderer* renderer, Project& project, ErrorPopup& errorPopup);
		bool isObjectDropdownBusy() noexcept;
	}
}