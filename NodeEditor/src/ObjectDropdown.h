#pragma once

#include <variant>

#include "NodeTabList.h"

namespace nv {
	namespace editor {
		class ObjectDropdown {
		private:
			nv::detail::TexturePtr m_tex;
			std::string m_texPath;
			int m_rowC = 1;
			int m_colC = 1;
			bool m_creatingSpritesheet = false;

			void showSpritesheetCreationWindow(SDL_Renderer* renderer, NodeEditor& currTab);
			void uploadSpriteSheet(SDL_Renderer* renderer, NodeEditor& currTab);
		public:
			void show(SDL_Renderer* renderer, NodeTabList& tabs);
			bool creatingSpritesheet() const noexcept;
		};
	}
}