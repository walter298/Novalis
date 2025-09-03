#pragma once

#include <optional>
#include <novalis/detail/memory/TexturePtr.h>

#include "EditedObjectData.h"
#include "ErrorPopup.h"
#include "FileID.h"

namespace nv {
	namespace editor {
		class ProjectFileManager;

		class SpritesheetCreator {
		private:
			nv::detail::TexturePtr m_tex;
			FileID m_fileID;
			int m_rowC = 1;
			int m_colC = 1;
		public:
			void init(nv::detail::TexturePtr tex, FileID fileID);
			std::optional<ObjectMetadata<Spritesheet>> show(SDL_Renderer* renderer, const ProjectFileManager& pfm, 
				bool& cancelled, ErrorPopup& errorPopup);
		};
	}
}