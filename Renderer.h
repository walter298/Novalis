#ifndef RENDERER_H
#define RENDERER_H

#include "DataUtil.h"

namespace nv {
	template<RenderObject... Objects>
	void renderCopy(SDL_Renderer* renderer, const Layers<Objects>&... objLayers) {
		auto renderImpl = [&](const auto& layers) {
			for (const auto& [layer, objLayer] : layers) {
				for (const auto& obj : objLayer) {
					obj.render(renderer);
				}
			}
		};
		((renderImpl(objLayers)), ...);
	}
}

#endif