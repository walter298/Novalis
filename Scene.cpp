#include "Scene.h"

void nv::Scene::loadSpriteClones(const json& j, SDL_Renderer* renderer) {
	//load all textures
	auto texPathLayers = j["texture_paths"].get<std::vector<std::vector<std::string>>>();
	std::vector<std::vector<nv::TexturePtr>> texLayers;

	auto createNewTexLayer = [&](const std::vector<std::string>& texPathLayer) {
		auto& currTexLayer = texLayers.emplace_back();
		currTexLayer.reserve(texPathLayers.size());
		for (const auto& texPath : texPathLayer) {
			currTexLayer.push_back(std::make_shared<nv::TextureDestructorWrapper>(IMG_LoadTexture(renderer, texPath.c_str())));
		}
	};
	texLayers.reserve(texPathLayers.size());
	for (const auto& texPathLayer : texPathLayers) {
		createNewTexLayer(texPathLayer);
	}

	//create textures
	auto spriteLayer = j["layer"].get<size_t>();
	auto spriteData  = j["sprite_data"].get<std::vector<Layers<TextureData>>>();

	size_t currDataLayer = 0;
	auto loadTexturesAndTexData = [&](Sprite::TextureLayers& texObjLayers, Layers<TextureData>& texDataLayers) {
		/*for (const auto [texLayer, texDataLayer] : views::zip(texLayers, texDataLayers)) {
			for (const auto [tex, texData] : views::zip(texLayer, texDataLayer)) {
				texObjLayers[currDataLayer].emplace_back(tex, std::move(texData));
			}
			currDataLayer++;
		}*/
	};
	for (auto& texDataLayers : spriteData) {
		nv::Sprite sprite;
		loadTexturesAndTexData(sprite.texObjLayers, texDataLayers);
		sprites[spriteLayer].push_back(std::move(sprite));
	}
}

nv::Scene::Scene(std::string_view absFilePath, SDL_Renderer* renderer) : m_renderer{ renderer }
{
	std::ifstream sceneFile{ absFilePath.data() };
	assert(sceneFile.is_open());

	auto json = json::parse(sceneFile);

	auto sprites = json["sprites"].array();

	for (const auto& sprite : sprites) {
		loadSpriteClones(sprite, renderer);
	}

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
		renderCopy(m_renderer, sprites, textures);
		SDL_RenderPresent(m_renderer);
	}
}