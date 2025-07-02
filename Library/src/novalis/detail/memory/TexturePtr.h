#pragma once

#include "../reflection/ClassIteration.h"

#include <SDL3/SDL_render.h>
#include <SDL3_image/SDL_image.h>

namespace nv {
	namespace detail {
		struct TexturePtr {
			SDL_Texture* tex = nullptr;

			TexturePtr() = default;
			TexturePtr(SDL_Renderer* renderer, const char* path);
			
			TexturePtr(const TexturePtr& other) noexcept : tex{ other.tex } {
				if (tex) {
					tex->refcount++;
				}
			}
			TexturePtr(TexturePtr&& other) noexcept : tex{ other.tex } {
				other.tex = nullptr;
			}
			TexturePtr& operator=(const TexturePtr& other) noexcept;
			TexturePtr& operator=(TexturePtr&& other) noexcept;

			~TexturePtr() noexcept {
				if (tex) {
					tex->refcount--;
					if (tex->refcount == 0) {
						SDL_DestroyTexture(tex); 
					}
				}
			}

			MAKE_INTROSPECTION(tex);
		};
	}
}