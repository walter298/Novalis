#include "Scene.h"

#include <thread>

nv::Scene::Scene(std::string_view absFilePath, SDL_Renderer* renderer) : m_renderer{ renderer } 
{
	std::ifstream sceneFile{ absFilePath.data() };
	assert(sceneFile.is_open());

	auto sceneJson = json::parse(sceneFile);

	auto loadObjects = [&, this](auto& objs, const json& objLayersRoot) {
		for (const auto& objLayer : objLayersRoot) {
			int layer = objLayer["layer"].get<int>();

			for (const auto& objJson : objLayer["objects"]) {
				objs[layer].emplace_back(renderer, objJson, m_texMap);
			}
		}
	};

	//load objects
	loadObjects(sprites, sceneJson["sprites"]);
	loadObjects(textures, sceneJson["texture_objects"]);

	sceneFile.close();
}

void nv::Scene::operator()() {
	running = true;

	eventHandler.addQuitEvent([this] { running = false; });

	constexpr auto FPS = 180;
	constexpr auto waitTime = 1000ms / FPS;

	while (running) {
		auto endTime = std::chrono::system_clock::now() + waitTime;

		eventHandler();
		
		const auto now = std::chrono::system_clock::now();
		if (now < endTime) {
			std::this_thread::sleep_for(endTime - now);
		}
		SDL_RenderClear(m_renderer);
		renderCopy(m_renderer, textures, sprites);
		SDL_RenderPresent(m_renderer);
	}
}