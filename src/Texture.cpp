#include "Texture.h"

nv::Texture::Texture(SDL_Texture* tex) noexcept
	: raw(tex) {}

nv::Texture::~Texture() noexcept {
	SDL_DestroyTexture(raw);
}

//void nv::from_json(const json& j, TexturePos& pos) {
//	pos.ren = j.at(TexturePos::renJkey).get<Rect>();
//	pos.world = j.at(TexturePos::worldJkey).get<Rect>();
//	pos.angle = j.at(TexturePos::angleJkey).get<size_t>();
//	pos.rotationPoint = j.at(TexturePos::rotationPointJkey).get<SDL_Point>();
//	pos.flip = j.at(TexturePos::flipJkey).get<SDL_RendererFlip>();
//}
//
//void nv::to_json(json& j, const TexturePos& pos) {
//
//}