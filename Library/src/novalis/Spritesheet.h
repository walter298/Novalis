#pragma once

#include <cassert>

#include "detail/memory/TexturePtr.h"
#include "detail/reflection/ClassIteration.h"
#include "detail/TextureData.h"

namespace nv {
	class Spritesheet : public detail::TextureRenderData {
	private:
		detail::TexturePtr m_tex;
		int m_rowC;
		int m_colC;
		int m_rowIdx = 0;
		int m_colIdx = 0;
	public:
		uint8_t opacity = 255;
		Spritesheet() = default;
		Spritesheet(detail::TexturePtr tex, int rows, int cols) noexcept;
		Spritesheet(detail::TexturePtr tex, detail::TextureRenderData rect, int rows, int cols) noexcept;

		int getRowCount() const noexcept {
			return m_rowC;
		}
		int getColumnCount() const noexcept {
			return m_colC;
		}
		int getRowIndex() const noexcept {
			return m_rowIdx;
		}
		int getColumnIndex() const noexcept {
			return m_colIdx;
		}

		void render(SDL_Renderer* renderer) const noexcept;
		void setTextureIndex(int row, int col) noexcept;
		void setOpacity(uint8_t opacityP) noexcept {
			opacity = opacityP;
		}
		MAKE_INTROSPECTION(m_tex, m_rowC, m_colC, m_rowIdx, m_colIdx, ren, world, screenRotationData, worldRotationData, flip)
	};
}