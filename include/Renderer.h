#ifndef RENDERER_H
#define RENDERER_H

#include <string>
#include <filesystem>

#include <imgui.h>
#include <imgui_impl_sdlrenderer2.h>

#include "DataUtil.h"

namespace nv {
	template<RenderObject... Objects>
	void renderCopy(SDL_Renderer* renderer, const Layers<Objects>&... objLayers) {
		auto renderImpl = [&](const auto& layers) {
			for (const auto& layer : layers) {
				for (const auto& object : layer) {
					object.render(renderer);
				}
			}
		};
		((renderImpl(objLayers)), ...);
	}

	namespace editor {
		template<RenderObject... Objects>
		void renderWithImGui(ImGuiIO& io, SDL_Renderer* renderer, const Layers<Objects>&... objLayers) {
			static constexpr ImVec4 color{ 0.45f, 0.55f, 0.60f, 1.00f };
			ImGui::Render();
			SDL_RenderSetScale(renderer, io.DisplayFramebufferScale.x, io.DisplayFramebufferScale.y);
			SDL_SetRenderDrawColor(renderer,
				//unfortunately SDL uses ints for screen pixels and ImGui uses floats 
				static_cast<Uint8>(color.x * 255), static_cast<Uint8>(color.y * 255),
				static_cast<Uint8>(color.z * 255), static_cast<Uint8>(color.w * 255));
			SDL_RenderClear(renderer);
			renderCopy(renderer, objLayers...);
			ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData());
			SDL_RenderPresent(renderer);
		}
	}
}

#endif