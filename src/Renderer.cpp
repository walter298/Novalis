#include "Renderer.h"

nv::Renderer::Renderer(SDL_Renderer* renderer) 
	: m_renderer(renderer)
{
}

void nv::Renderer::move(int dx, int dy) noexcept {
	m_background->ren.move(dx, dy);

	auto moveSprites = [dx, dy](auto& sprites) {
		for (auto& [layer, sprites] : sprites) {
			for (auto& sprite : sprites) {
				sprite->renMove(dx, dy);
			}
		}
	};
	moveSprites(m_sprites);
	moveSprites(m_multiSprites);
}

void nv::Renderer::clear() noexcept {
	m_sprites.clear();
}

void nv::Renderer::setBackground(Background* background) noexcept {
	m_background = background;
}

void nv::Renderer::addSprite(Sprite* sprite, int layer) {
	m_sprites[layer].insert(sprite);
}
void nv::Renderer::removeSprite(const Sprite* const sprite, int layer) {
	removeSpriteImpl(m_sprites, sprite->getID(), layer);
}

void nv::Renderer::addSprite(MultitextureSprite* sprite, int layer) {
	m_multiSprites[layer].insert(sprite);
}
void nv::Renderer::removeSprite(const MultitextureSprite* const sprite, int layer) {
	removeSpriteImpl(m_multiSprites, sprite->getID(), layer);
}

void nv::Renderer::render() noexcept {
	SDL_RenderClear(m_renderer);
	m_background->render(m_renderer);
	for (auto& [layer, objs] : m_sprites) {
		for (auto& obj : objs) {
			obj->render(m_renderer);
		}
	}
	SDL_RenderPresent(m_renderer);
}
