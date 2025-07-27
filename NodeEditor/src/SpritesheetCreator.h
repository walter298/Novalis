#pragma once

#include <optional>
#include <novalis/detail/memory/TexturePtr.h>

#include "EditedObjectData.h"
#include "ErrorPopup.h"
#include "FileID.h"

namespace nv {
	namespace editor {
		class VirtualFilesystem;

		class SpritesheetCreator {
		private:
			nv::detail::TexturePtr m_tex;
			FileID m_fileID;
			int m_rowC = 1;
			int m_colC = 1;
		public:
			void init(nv::detail::TexturePtr tex, FileID fileID);
			std::optional<EditedObjectData<Spritesheet>> show(SDL_Renderer* renderer, VirtualFilesystem& vfs, 
				bool& cancelled, ErrorPopup& errorPopup);
		};
	}
}