#include "Spritesheet.h"

nv::Spritesheet::Spritesheet(detail::TexturePtr tex, int rows, int cols) noexcept
	: m_tex{ tex }, m_rowC{ rows }, m_colC{ cols }
{
	assert(m_rowC > 0 && m_colC > 0);

	//get image dimensions
	float imageW, imageH;
	SDL_GetTextureSize(tex.tex, &imageW, &imageH);

	//make texture size proportional to the # of rows and columns
	ren.size.x = imageW / static_cast<float>(cols);
	ren.size.y = imageH / static_cast<float>(rows);
	world.size = ren.size;
}

nv::Spritesheet::Spritesheet(detail::TexturePtr tex, detail::TextureRenderData renderData, int rows, int cols) noexcept
	: detail::TextureRenderData{ renderData }, m_tex{ tex }, m_rowC{ rows }, m_colC{ cols }
{
	assert(m_rowC > 0 && m_colC > 0);
}

void nv::Spritesheet::render(SDL_Renderer* renderer) const noexcept {
	auto quadrantWidth = m_tex.tex->w / static_cast<float>(m_colC);
	auto quadrantHeight = m_tex.tex->h / static_cast<float>(m_rowC);
	SDL_FRect srcRect{
		m_colIdx * quadrantWidth, m_rowIdx * quadrantHeight,
		quadrantWidth, quadrantHeight
	};
	auto destRect = ren.sdlRect();
	auto angle = static_cast<double>(screenRotationData.angle);
	SDL_FPoint p = screenRotationData.rotationPoint;
	SDL_SetTextureAlphaMod(m_tex.tex, opacity);
	SDL_RenderTextureRotated(renderer, m_tex.tex, &srcRect, &destRect, angle, &p, flip);
}

void nv::Spritesheet::setTextureIndex(int row, int col) noexcept {
	assert(row < m_rowC);
	assert(col < m_colC);
	m_rowIdx = row;
	m_colIdx = col;
}
