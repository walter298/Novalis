#include "TexturePtr.h"

#include <cassert>
#include <format>

struct FormattedException : public std::exception {
	std::string msg;

	FormattedException(std::string msg) : msg{ std::move(msg) }
	{
	}
	const char* what() const noexcept override {
		return msg.c_str();
	}
};

nv::detail::TexturePtr::TexturePtr(SDL_Renderer* renderer, const char* path)
{
	tex = IMG_LoadTexture(renderer, path);
	if (tex == nullptr) {
		throw FormattedException{ std::format("Error: could not load texture at path {}: {}", path, SDL_GetError()) };
	}
}

nv::detail::TexturePtr::TexturePtr(SDL_Texture* tex) noexcept : tex{ tex }
{
	assert(tex);
	assert(tex->refcount == 1);
}

static void release(SDL_Texture* tex) {
	if (tex) {
		assert(tex->refcount > 0);
		tex->refcount--;
		if (tex->refcount == 0) {
			SDL_DestroyTexture(tex);
		}
	}
}

nv::detail::TexturePtr& nv::detail::TexturePtr::operator=(const TexturePtr& other) noexcept
{
	assert(&other != this);
	release(tex);
	tex = other.tex;
	if (tex) {
		tex->refcount++;
	}
	return *this;
}

nv::detail::TexturePtr& nv::detail::TexturePtr::operator=(TexturePtr&& other) noexcept
{
	assert(&other != this);
	release(tex);
	tex = other.tex;
	other.tex = nullptr;
	return *this;
}

void nv::detail::TexturePtr::destroy() noexcept {
	release(tex);
	tex = nullptr;
}
