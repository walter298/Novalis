#include "Scene.h"

nv::Scene::Scene(std::string_view absFilePath, Instance& instance) : renderer{ instance.getRawRenderer() } {
	std::ifstream sceneFile{ absFilePath.data() };
	assert(sceneFile.is_open());

	auto jsonData = json::parse(sceneFile);

	//load sprites
	//auto spriteData = jsonData["sprite_names"].get<SpriteData>();
	//for (const auto& [spriteName, layers] : spriteData) {
	//	auto layers = spriteName;
	//}
	//auto objectNames = jsonData["sprites"].get<std::vector<std::string>>();
	//for (const auto& name : objectNames) {
	//	//auto sprite = instance.getSprite(name);

	//	auto ren = jsonData.at("sprites").at(name).at("layer").get<Rect>();
	//	/*sprite.pos.ren.setPos(ren.rect.x, ren.rect.y);
	//	sprite.pos.ren.setSize(ren.rect.w, ren.rect.h);

	//	auto world = jsonData.at("sprites")[name]["world"].get<Rect>();
	//	sprite.pos.world.setPos(world.rect.x, world.rect.y);
	//	sprite.pos.ren.setSize(world.rect.w, world.rect.h);*/

	//	auto layer = jsonData.at("sprites").at(name).at("layer").get<int>();
	//	//renderer.add(&sprite, layer);
	//}
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
		renderer.render();
	}
}